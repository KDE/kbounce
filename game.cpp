/*
 * Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdlib.h>
#include <qwidget.h>
#include <qkeycode.h>
#include <qpainter.h>
#include <qtimer.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qevent.h>

#include "game.h"

#define TILE_SIZE 16

#define TILE_FREE 0
#define TILE_BORDER 1
#define TILE_WALLHOR 2
#define TILE_WALLVER 3


Ball::Ball(QCanvasPixmapArray* array, QCanvas* canvas)
   : QCanvasSprite( array, canvas )
{
}

void Ball::advance(int stage)
{
   bool reflectX = false;
   bool reflectY = false;

   // balls already on a wall will be stopped (should normally never happen)
   if ( collide(0, 0) ) setVelocity( 0, 0 );

   // check for collisions
   if ( collide(xVelocity(), 0) ) reflectX = true;
   if ( collide(0, yVelocity()) ) reflectY = true;
   if ( !reflectX && !reflectY && collide(xVelocity(), yVelocity()) ) reflectX = reflectY = true;

   if ( reflectX ) setXVelocity( -xVelocity() );
   if ( reflectY ) setYVelocity( -yVelocity() );

   QCanvasSprite::advance( stage );
}

bool Ball::collide( double dx, double dy )
{
   QRect r = boundingRect();
   r.moveBy( dx, dy );
   JezzField* field = (JezzField *)canvas();
   
   int ul = field->tile( r.left() / TILE_SIZE, r.top() / TILE_SIZE );
   int ur = field->tile( r.right() / TILE_SIZE, r.top() / TILE_SIZE );
   int bl = field->tile( r.left() / TILE_SIZE, r.bottom() / TILE_SIZE );
   int br = field->tile( r.right() / TILE_SIZE, r.bottom() / TILE_SIZE );

   if ( ul!=TILE_FREE ) field->emitBallCollisiton( this, r.left() / TILE_SIZE, r.top() / TILE_SIZE, ul );
   if ( ur!=TILE_FREE ) field->emitBallCollisiton( this, r.right() / TILE_SIZE, r.top() / TILE_SIZE, ur );
   if ( bl!=TILE_FREE ) field->emitBallCollisiton( this, r.left() / TILE_SIZE, r.bottom() / TILE_SIZE, bl );
   if ( br!=TILE_FREE ) field->emitBallCollisiton( this, r.right() / TILE_SIZE, r.bottom() / TILE_SIZE, br );
      
   return ( ul!=TILE_FREE || ur!=TILE_FREE || bl!=TILE_FREE || br!=TILE_FREE );
}

/*************************************************************************/

Wall::Wall( JezzField *field, int x, int y, Direction dir, QObject *parent, const char *name )
   : QObject( parent, name ), m_dir( dir ), m_field( field ), m_startX( x ), m_startY( y )
{
   kdDebug() << "Wall::Wall" << endl;
   m_dx = 0;
   m_dy = 0;
   switch ( m_dir )
   {
      case Up: m_dy = -1; m_tile = TILE_WALLVER;break;
      case Down: m_dy = 1; m_tile = TILE_WALLVER; break;
      case Left: m_dx = -1; m_tile = TILE_WALLHOR; break;
      case Right: m_dx = 1; m_tile = TILE_WALLHOR; break;
   }

   m_x = m_startX;
   m_y = m_startY;

   m_timer = new QTimer( this );
   m_timer->start( 100 );
   connect( m_timer, SIGNAL(timeout()), this, SLOT(update()) );
}

void Wall::finish()
{
   m_timer->stop();
   emit finished( this );
}

void Wall::update()
{ 
   kdDebug() << "Wall::update" << endl;
   m_field->setTile( m_x, m_y, m_tile );

   m_x += m_dx;
   m_y += m_dy;

   if ( m_field->tile(m_x, m_y)==TILE_FREE )
   {
      // check whether there is a ball at the moment
      QCanvasItemList cols = m_field->collisions( QRect(m_x*TILE_SIZE, m_y*TILE_SIZE, TILE_SIZE, TILE_SIZE) );
      if ( cols.count()>0 )
      {
	 kdDebug() << "Wall corner collision" << endl;
	 finish();		 
      } else
	 m_field->setTile( m_x, m_y, TILE_BORDER );
   } else
      finish();
}

void Wall::black()
{
   if ( m_dx )
   {      
      for ( int x=m_startX ; x!=m_x; x+=m_dx )
	 m_field->setTile( x, m_startY, TILE_BORDER );
   } else
   {
      for ( int y=m_startY ; y!=m_y; y+=m_dy )
	 m_field->setTile( m_startX, y, TILE_BORDER );
   }
}

/*************************************************************************/

JezzField::JezzField( QObject* parent, const char* name )
   : QCanvas( parent, name )
{
}

/*************************************************************************/

JezzView::JezzView(QCanvas* viewing, QWidget* parent, const char* name, WFlags f)
   : QCanvasView( viewing, parent, name, f ), m_vertical( false )
{
   setResizePolicy( AutoOne );
   setHScrollBarMode( AlwaysOff );
   setVScrollBarMode( AlwaysOff );  

   setCursor( sizeHorCursor );
}

void JezzView::viewportMouseReleaseEvent( QMouseEvent *ev )
{
   kdDebug() << "Mouse click" << endl;
   if ( ev->button() & RightButton )
   {
      kdDebug() << "Right mouse click" << endl;
      m_vertical = !m_vertical;
      if ( m_vertical ) setCursor( sizeVerCursor ); else setCursor( sizeHorCursor );
   }
   
   if ( ev->button() & LeftButton )
   {
      kdDebug() << "Left mouse click" << endl;
      emit buildWall( ev->x()/TILE_SIZE, ev->y()/TILE_SIZE, m_vertical );
   }
}

/*************************************************************************/

JezzGame::JezzGame( int ballNum, QWidget *parent, char *name )
   : QWidget( parent, name ), m_wall1( 0 ), m_wall2( 0 ), m_blackTiles( 0 )
{
   QString path = kapp->dirs()->findResourceDir( "data", "kjezz/pics/ball0000.png" ) + "kjezz/pics/";

   // load gfx
   m_ballPixmaps = new QCanvasPixmapArray( path + "ball0000.png", 1 );
   QPixmap tiles( path+"tiles.png" );

   // create field
   m_field = new JezzField( this, "m_field" );
   m_field->resize( TILE_SIZE*FIELD_WIDTH, TILE_SIZE*FIELD_HEIGHT );
   m_field->setTiles( tiles, FIELD_WIDTH, FIELD_HEIGHT, TILE_SIZE, TILE_SIZE );

   for ( int x=0; x<FIELD_WIDTH; x++ )
     	 m_field->setTile( x, 0, TILE_BORDER );
   for ( int y=1; y<FIELD_HEIGHT-1; y++ )
   {
      m_field->setTile( 0, y, TILE_BORDER );
      for ( int x=1; x<FIELD_WIDTH-1; x++ )
     	 m_field->setTile( x, y, TILE_FREE );
      m_field->setTile( FIELD_WIDTH-1, y, TILE_BORDER );
   }
   for ( int x=0; x<FIELD_WIDTH; x++ )
     	 m_field->setTile( x, FIELD_HEIGHT-1, TILE_BORDER );

   connect( m_field, SIGNAL(ballCollision(Ball *, int, int, int)), this, SLOT(ballCollision(Ball *, int, int, int)) );

   // create view
   QHBoxLayout *layout = new QHBoxLayout( this );
   m_view = new JezzView( m_field, this, "m_view" );
   connect( m_view, SIGNAL(buildWall(int, int, bool)), this, SLOT(buildWall(int, int, bool)) );
   layout->addWidget( m_view );

   // create balls
   for ( int n=0; n<ballNum; n++ )
   {
      Ball *ball = new Ball( m_ballPixmaps, m_field );
      m_balls.append( ball );
      ball->setVelocity( ((rand() & 1)*2-1)*2, ((rand() & 1)*2-1)*1 );
      ball->move( 80+rand()%480, 80+rand()%320 );
   }

   // create game clock
   m_clock = new QTimer( this );
   connect( m_clock, SIGNAL(timeout()), this, SLOT(tick()) );
   m_clock->start( 10 );
}

JezzGame::~JezzGame()
{
   m_balls.clear();
}

void JezzGame::makeBlack()
{
   // copy current field into buffer
   for ( int y=0; y<FIELD_HEIGHT; y++ )
      for ( int x=0; x<FIELD_WIDTH; x++ )
	 m_buf[x][y] = m_field->tile( x, y );

   // fill areas that contains a ball
   for ( Ball *ball=m_balls.first(); ball!=0; ball=m_balls.next() )
      fill( ball->x()/TILE_SIZE, ball->y()/TILE_SIZE );

   // areas still free can be blacked now   
   for ( int y=0; y<FIELD_HEIGHT; y++ )
      for ( int x=0; x<FIELD_WIDTH; x++ )
      {
	 if ( m_buf[x][y]==TILE_FREE )
	    m_field->setTile( x, y, TILE_BORDER );
      }
}

// not very optimized yet, but most important is, that it works
void JezzGame::fill( int x, int y )
{
   if ( m_buf[x][y]!=TILE_FREE) return; 
 
   // go left
   int _x=x;
   for ( ; m_buf[_x][y]==TILE_FREE; _x-- )
      m_buf[_x][y] = TILE_BORDER;      
   int stopx = _x;

   // fill above   
   for ( _x=x; _x>stopx; _x-- )
      if ( m_buf[_x][y-1]==TILE_FREE ) fill( _x, y-1 );
   
   // fill below
   for ( _x=x; _x>stopx; _x-- )
      if ( m_buf[_x][y+1]==TILE_FREE ) fill( _x, y+1 );
      
   // go right
   for ( _x=x+1; m_buf[_x][y]==TILE_FREE; _x++ )
      m_buf[_x][y] = TILE_BORDER;
   stopx = _x;

   // fill above
   for ( _x=x+1; _x<stopx; _x++ )
      if ( m_buf[_x][y-1]==TILE_FREE ) fill( _x, y-1 );
   
   // fill below;
   for ( _x=x+1; _x<stopx; _x++ )
      if ( m_buf[_x][y+1]==TILE_FREE ) fill( _x, y+1 );   
}

void JezzGame::ballCollision( Ball */*ball*/, int /*x*/, int /*y*/, int tile )
{
   if ( tile!=TILE_BORDER )
   {
      kdDebug() << "Collision" << endl;
   }
}

void JezzGame::buildWall( int x, int y, bool vertical )
{
   kdDebug() << "JezzGame::buildWall( x=" << x << " y=" << y << " vertical=" << vertical << " )" << endl;
   if ( m_field->tile(x, y)==TILE_FREE )
   {
      // check whether there is a ball at the moment
      QCanvasItemList cols = m_field->collisions( QRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE) );
      if ( cols.count()>0 )
      {
	 kdDebug() << "Direct collision" << endl;
	 emit ballCollision( (Ball*)cols.first(), x, y, TILE_WALLHOR );
	 return;	
      }
      
      // start walls
      if ( !m_wall1 )
      {
	 m_wall1 = new Wall( m_field, x, y, vertical?Wall::Up:Wall::Left, this, "m_wall1" );
	 connect( m_wall1, SIGNAL(finished(Wall *)), this, SLOT(wallFinished(Wall *)) );
      }

      if ( !m_wall2 ) 
      {
	 m_wall2 = new Wall( m_field, x, y, vertical?Wall::Down:Wall::Right, this, "m_wall2" );
	 connect( m_wall2, SIGNAL(finished(Wall *)), this, SLOT(wallFinished(Wall *)) );
      }
   }
}

void JezzGame::wallFinished( Wall *wall )
{
   kdDebug() << "JezzGame::wallFinished" << endl;
   if ( m_wall1==wall )
   {
      m_wall1->black();
      delete m_wall1;
      m_wall1 = 0;
   }

   if ( m_wall2==wall )
   {
      m_wall2->black();
      delete m_wall2;
      m_wall2 = 0;
   }

   makeBlack();
}

void JezzGame::tick()
{ 
   m_field->advance();
   kapp->syncX();
}

#include "game.moc"
