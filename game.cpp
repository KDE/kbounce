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
#include <qpushbutton.h>
#include <qfont.h>

#include "game.h"


#define TILE_SIZE 16

#define TILE_FREE 0
#define TILE_BORDER 1
#define TILE_WALLEND 2
#define TILE_WALLUP 3
#define TILE_WALLDOWN 4
#define TILE_WALLLEFT 5
#define TILE_WALLRIGHT 6

#define GAME_DELAY 15
#define BALL_ANIM_DELAY 60
#define WALL_DELAY 100


SimpleSoundServer *JezzGame::m_artsServer = 0;
QString JezzGame::m_soundPath;

#define MS2TICKS( ms ) ((ms)/GAME_DELAY)

Ball::Ball(QCanvasPixmapArray* array, QCanvas* canvas)
    : QCanvasSprite( array, canvas ), m_animDelay( 0 ), m_soundDelay( MS2TICKS(BALL_ANIM_DELAY)/2 )
{
}

void Ball::update()
{
   // set pixmap frame
   m_animDelay--;
   if ( m_animDelay<=0 )
   {
       m_animDelay =  MS2TICKS(BALL_ANIM_DELAY);
       int frameNum = frame();
       frameNum++;
       if ( frameNum>=frameCount() )
           frameNum = 0;
       setFrame( frameNum );
   }
}

void Ball::advance(int stage)
{
   bool reflectX = false;
   bool reflectY = false;

   m_soundDelay++;

   // balls already on a wall will be stopped (should normally never happen)
   if ( collide(0, 0) ) setVelocity( 0, 0 );

   // check for collisions
   if ( collide(xVelocity(), 0) ) reflectX = true;
   if ( collide(0, yVelocity()) ) reflectY = true;
   if ( !reflectX && !reflectY && collide(xVelocity(), yVelocity()) ) reflectX = reflectY = true;

   // emit collision
   QRect r = boundingRect();
   r.moveBy( xVelocity(), yVelocity() );
   JezzField* field = (JezzField *)canvas();

   int ul = field->tile( r.left() / TILE_SIZE, r.top() / TILE_SIZE );
   int ur = field->tile( r.right() / TILE_SIZE, r.top() / TILE_SIZE );
   int bl = field->tile( r.left() / TILE_SIZE, r.bottom() / TILE_SIZE );
   int br = field->tile( r.right() / TILE_SIZE, r.bottom() / TILE_SIZE );

   if ( ul!=TILE_FREE ) field->emitBallCollisiton( this, r.left() / TILE_SIZE, r.top() / TILE_SIZE, ul ); else
   if ( ur!=TILE_FREE ) field->emitBallCollisiton( this, r.right() / TILE_SIZE, r.top() / TILE_SIZE, ur ); else
   if ( bl!=TILE_FREE ) field->emitBallCollisiton( this, r.left() / TILE_SIZE, r.bottom() / TILE_SIZE, bl ); else
   if ( br!=TILE_FREE ) field->emitBallCollisiton( this, r.right() / TILE_SIZE, r.bottom() / TILE_SIZE, br );

   // apply reflection
   if ( reflectX ) setXVelocity( -xVelocity() );
   if ( reflectY ) setYVelocity( -yVelocity() );

   // play collision sound
   if ( reflectX || reflectY )
   {
       if ( m_soundDelay>50 ) JezzGame::playSound( "reflect.au" );
       m_soundDelay = 0;
   }

   // update field
   update();
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

   return ( ul!=TILE_FREE || ur!=TILE_FREE || bl!=TILE_FREE || br!=TILE_FREE );
}

/*************************************************************************/

Wall::Wall( JezzField *field, int x, int y, Direction dir, int tile, QObject *parent, const char *name )
    : QObject( parent, name ), m_dir( dir ), m_field( field ), m_startX( x ), m_startY( y ),
      m_tile( tile ), m_delay( MS2TICKS(WALL_DELAY)/2 ), m_active( true )
{
   kdDebug() << "Wall::Wall" << endl;

   // setup position and direction
   m_dx = 0;
   m_dy = 0;
   switch ( m_dir )
   {
      case Up: m_dy = -1; break;
      case Down: m_dy = 1; break;
      case Left: m_dx = -1; break;
      case Right: m_dx = 1; break;
   }

   m_x = m_startX;
   m_y = m_startY;

   m_field->setTile( m_x, m_y, m_tile );
}

void Wall::finish()
{
    m_active = false;
}

bool Wall::isFree( int x, int y )
{
    if ( m_field->tile(x, y)==TILE_FREE )
    {
        // check whether there is a ball at the moment
        QCanvasItemList cols = m_field->collisions( QRect(x*TILE_SIZE, y*TILE_SIZE,
                                                          TILE_SIZE, TILE_SIZE) );
        if ( cols.count()==0 )
            return true;
    }

    return false;
}

void Wall::update()
{
}

void Wall::advance()
{
    update();

    // move wall
    if ( m_active )
    {
        m_delay--;
        if ( m_delay<=0 )
        {
            m_delay =  MS2TICKS(WALL_DELAY);

            // set previous tile
            m_field->setTile( m_x, m_y, m_tile );

            // check whether next place is still free
            if ( isFree(m_x+m_dx, m_y+m_dy) )
            {
                // move ball
                m_x += m_dx;
                m_y += m_dy;

                // set tile
                m_field->setTile( m_x, m_y, TILE_WALLEND );
            } else
            {
                finish();
                emit finished( this, m_field->tile( m_x+m_dx, m_y+m_dy ) );
            }
        }
    }
}

void Wall::fill( int tile )
{
   if ( m_dx )
   {
      for ( int x=m_startX ; x!=m_x; x+=m_dx )
          if ( m_field->tile(x, m_startY)==m_tile ) m_field->setTile( x, m_startY, tile );

      m_field->setTile( m_x, m_startY, tile );
   } else
   {
      for ( int y=m_startY ; y!=m_y; y+=m_dy )
         if ( m_field->tile(m_startX, y)==m_tile ) m_field->setTile( m_startX, y, tile );

      m_field->setTile( m_startX, m_y, tile );
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
   if ( ev->button() & RightButton )
   {
      m_vertical = !m_vertical;
      if ( m_vertical ) setCursor( sizeVerCursor ); else setCursor( sizeHorCursor );
   }

   if ( ev->button() & LeftButton )
   {
      emit buildWall( ev->x()/TILE_SIZE, ev->y()/TILE_SIZE, m_vertical );
   }
}

/*************************************************************************/

JezzGame::JezzGame( int ballNum, QWidget *parent, const char *name )
    : QWidget( parent, name ), m_wall1( 0 ), m_wall2( 0 ),
      m_text( 0 ), m_running( false ), m_percent( 0 )
{
   QString path = kapp->dirs()->findResourceDir( "data", "kjezz/pics/ball0000.png" ) + "kjezz/pics/";

   // load gfx
   m_ballPixmaps = new QCanvasPixmapArray( path + "ball%1.png", 25 );
   for ( unsigned n=0; n<m_ballPixmaps->count(); n++ )
       m_ballPixmaps->image(n)->setOffset( 0, 0 );
   QPixmap tiles( path+"tiles.png" );

   // setup arts
   m_artsServer = new SimpleSoundServer;
   *m_artsServer = Arts::Reference("global:Arts_SimpleSoundServer");
   if ( m_artsServer->isNull() )
       kdDebug() << "Can't connect to aRts sound server" << endl;
   m_soundPath = kapp->dirs()->findResourceDir( "data", "kjezz/sounds/death.au" ) +
                 "kjezz/sounds/";

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
   m_view = new JezzView( m_field, this, "m_view" );
   m_view->move( 0, 0 );
   m_view->adjustSize();
   connect( m_view, SIGNAL(buildWall(int, int, bool)), this, SLOT(buildWall(int, int, bool)) );

   // create balls
   for ( int n=0; n<ballNum; n++ )
   {
      Ball *ball = new Ball( m_ballPixmaps, m_field );
      m_balls.append( ball );
      ball->setVelocity( ((rand() & 1)*2-1)*2, ((rand() & 1)*2-1)*2 );
      ball->setFrame( rand() % 25 );
      ball->move( 4*TILE_SIZE + rand() % ( (FIELD_WIDTH-8)*TILE_SIZE ),
                  4*TILE_SIZE + rand() % ( (FIELD_HEIGHT-8)*TILE_SIZE ) );
      ball->show();
   }

   // create text label
   m_text = new QCanvasText( m_field );

   // create game clock
   m_clock = new QTimer( this );
   connect( m_clock, SIGNAL(timeout()), this, SLOT(tick()) );
   m_clock->start( GAME_DELAY );

   // setup geometry
   setFixedSize( m_view->size() );
}

JezzGame::~JezzGame()
{
    m_balls.clear();
    delete m_view;
    delete m_field;
    delete m_ballPixmaps;
    delete m_artsServer;
}

void JezzGame::display( QString text, int size )
{
    if ( !text.isEmpty() )
    {
        kdDebug() << "text = " << text << endl;

        QFont font( "Helvetica", size, QFont::Bold );
        font.setStyleHint( QFont::Helvetica );
        m_text->setFont( font );
        m_text->setText( text );

        QRect size = m_text->boundingRect();
        m_text->move( ( FIELD_WIDTH*TILE_SIZE - size.width() ) / 2,
                      ( FIELD_HEIGHT*TILE_SIZE - size.height() ) / 2 );

        m_text->show();
    } else
    {
        m_text->hide();
    }
}

void JezzGame::playSound( QString name )
{
    if( !m_artsServer->isNull() )
    {
        QString path = m_soundPath + name;
        m_artsServer->play( path.latin1() );
    }
}

void JezzGame::start()
{
    m_running = true;
}

void JezzGame::stop()
{
    m_running = false;
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

   m_field->update();
   m_view->repaint();

   // count percent value of occupied area
   int p = percent();
   if ( p!=m_percent )
   {
       m_percent = p;
       emit newPercent( m_percent );
   }
}

int JezzGame::percent()
{
   int notFree = 0;
   for ( int y=1; y<FIELD_HEIGHT-1; y++ )
      for ( int x=1; x<FIELD_WIDTH-1; x++ )
      {
         if ( m_field->tile(x,y)!=TILE_FREE )
             notFree++;
      }

   return 100 * notFree / ( (FIELD_WIDTH-1) * (FIELD_HEIGHT-1) );
}

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
   if ( tile!=TILE_BORDER && tile!=TILE_WALLEND )
   {
      kdDebug() << "Collision" << endl;

      // play explosion sound
      playSound( "death.au" );

      // stop walls
      if ( (tile==TILE_WALLUP || tile==TILE_WALLLEFT) && m_wall1 )
      {
          m_wall1->finish();
          m_wall1->fill( TILE_FREE );
          delete m_wall1;
          m_wall1 = 0;
      }

      if ( (tile==TILE_WALLDOWN || tile==TILE_WALLRIGHT) && m_wall2 )
      {
          m_wall2->finish();
          m_wall2->fill( TILE_FREE );
          delete m_wall2;
          m_wall2 = 0;
      }

      // update view
      m_field->update();
      m_view->repaint();

      // send death msg
      emit died();
   }
}

void JezzGame::buildWall( int x, int y, bool vertical )
{
    if ( !m_running ) return;

   kdDebug() << "JezzGame::buildWall( x=" << x << " y=" << y << " vertical=" << vertical << " )" << endl;
   if ( m_field->tile(x, y)==TILE_FREE )
   {
       playSound( "wallstart.au" );

      // check whether there is a ball at the moment
      QCanvasItemList cols = m_field->collisions( QRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE) );
      if ( cols.count()>0 )
      {
         kdDebug() << "Direct collision" << endl;
         emit ballCollision( (Ball*)cols.first(), x, y, TILE_WALLUP );
         return;
      }

      // start walls
      if ( !m_wall1 )
      {
         m_wall1 = new Wall( m_field, x, y,
                             vertical? Wall::Up : Wall::Left,
                             vertical? TILE_WALLUP : TILE_WALLLEFT,
                             this, "m_wall1" );
         connect( m_wall1, SIGNAL(finished(Wall *, int)),
                  this, SLOT(wallFinished(Wall *, int)) );            }

      if ( !m_wall2 )
      {
         m_wall2 = new Wall( m_field, x, y,
                             vertical? Wall::Down: Wall::Right,
                             vertical? TILE_WALLDOWN : TILE_WALLRIGHT,
                             this, "m_wall2" );
         connect( m_wall2, SIGNAL(finished(Wall *, int)),
                  this, SLOT(wallFinished(Wall *, int)) );
      }
   }
}

void JezzGame::wallFinished( Wall *wall, int tile )
{
    playSound( "wallend.au" );

    if ( tile==TILE_WALLEND )
    {
        if ( m_wall1 )
        {
            m_wall1->fill( TILE_FREE );
            delete m_wall1;
            m_wall1 = 0;
        }

        if ( m_wall2 )
        {
            m_wall2->fill( TILE_FREE );
            delete m_wall2;
            m_wall2 = 0;
        }
    } else
    {
        if ( m_wall1==wall && m_wall1 )
        {
            m_wall1->fill( TILE_BORDER );
            delete m_wall1;
            m_wall1 = 0;
        }

        if ( m_wall2==wall && m_wall2 )
        {
            m_wall2->fill( TILE_BORDER );
            delete m_wall2;
            m_wall2 = 0;
        }
    }

    m_field->update();
    m_view->repaint();

    makeBlack();
}

void JezzGame::tick()
{
    if ( m_running )
    {
        if ( m_field ) m_field->advance();
        if ( m_wall1 ) m_wall1->advance();
        if ( m_wall2 ) m_wall2->advance();
    } else
    {
        for ( Ball *ball=m_balls.first(); ball!=0; ball=m_balls.next() )
            ball->update();

        if ( m_field ) m_field->update();
        if ( m_wall1 ) m_wall1->update();
        if ( m_wall2 ) m_wall2->update();
    }

    //kapp->syncX();
}

#include "game.moc"
