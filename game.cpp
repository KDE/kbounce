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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <stdlib.h>
#include <qtimer.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <qimage.h>
#include <kglobalsettings.h>

#include "game.h"


#define TILE_SIZE 16

#define TILE_FIRST ((FIELD_WIDTH-2)*(FIELD_HEIGHT-2))
#define TILE_FREE (TILE_FIRST + 0)
#define TILE_BORDER (TILE_FIRST + 1)
#define TILE_WALLEND (TILE_FIRST + 2)
#define TILE_WALLUP (TILE_FIRST + 3)
#define TILE_WALLDOWN (TILE_FIRST + 4)
#define TILE_WALLLEFT (TILE_FIRST + 5)
#define TILE_WALLRIGHT (TILE_FIRST + 6)

#define GAME_DELAY 15
#define BALL_ANIM_DELAY 60
#define WALL_DELAY 100


#if HAVE_ARTS
SimpleSoundServer *JezzGame::m_artsServer = 0;
#endif
QString JezzGame::m_soundPath;
bool JezzGame::m_sound = true;

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

   // ball already on a wall? (should normally never happen)
   // commented out to stop bug which causes balls to
   // sometimes stop when clicked on
   // if ( collide(0, 0) ) setVelocity( 0, 0 );

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
   //kdDebug(12008) << "Wall::Wall" << endl;

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

void Wall::fill( bool black )
{
   if ( m_dx )
   {
      for ( int x=m_startX ; x!=m_x; x+=m_dx )
          if ( m_field->tile(x, m_startY)==m_tile )
              m_field->setGameTile( x, m_startY, black );

      m_field->setGameTile( m_x, m_startY, black );
   } else
   {
      for ( int y=m_startY ; y!=m_y; y+=m_dy )
         if ( m_field->tile(m_startX, y)==m_tile )
             m_field->setGameTile( m_startX, y, black );

      m_field->setGameTile( m_startX, m_y, black );
   }
}

/*************************************************************************/

JezzField::JezzField( const QPixmap &tiles, const QPixmap &background, QObject* parent, const char* name )
    : QCanvas( parent, name ), m_tiles( tiles )
{
    setPixmaps( tiles, background );
}

void JezzField::setGameTile( int x, int y, bool black )
{
    if ( m_background )
        setTile( x, y, black ? ((x-1)+(y-1)*(FIELD_WIDTH-2)) : TILE_FREE );
    else
        setTile( x, y, black ? TILE_BORDER : TILE_FREE );
}

void JezzField::setBackground( const QPixmap &background )
{
    // copy current field into buffer
    int backup[FIELD_WIDTH][FIELD_HEIGHT];
    for ( int y=0; y<FIELD_HEIGHT; y++ )
        for ( int x=0; x<FIELD_WIDTH; x++ )
            backup[x][y] = tile( x, y );

    setPixmaps( m_tiles, background );

    // restore tiles
    for ( int x=0; x<FIELD_WIDTH; x++ )
        setTile( x, 0, TILE_BORDER );
    for ( int y=1; y<FIELD_HEIGHT-1; y++ ) {

        setTile( 0, y, TILE_BORDER );

        for ( int x=1; x<FIELD_WIDTH-1; x++ ) {
            int tile = backup[x][y];

            if ( m_background ) {
                if ( tile==TILE_BORDER || tile<TILE_FIRST )
                    tile = (x-1)+(y-1)*(FIELD_WIDTH-2);
            } else {
                if ( tile<TILE_FIRST )
                    tile = TILE_BORDER;
            }

            setTile( x, y, tile );
        }

        setTile( FIELD_WIDTH-1, y, TILE_BORDER );
    }
    for ( int x=0; x<FIELD_WIDTH; x++ )
        setTile( x, FIELD_HEIGHT-1, TILE_BORDER );
}

void JezzField::setPixmaps( const QPixmap &tiles, const QPixmap &background )
{
    // create new tiles
    QPixmap allTiles( TILE_SIZE*(FIELD_WIDTH-2), TILE_SIZE*(FIELD_HEIGHT-1) );

    if ( background.width()==0 || background.height()==0 ) {
        m_background = false;
    } else {
        // handle background
        m_background = true;
        QImage img = background.convertToImage();
        QPixmap scalledBackground( img.smoothScale( TILE_SIZE*(FIELD_WIDTH-2),
                                                      TILE_SIZE*(FIELD_HEIGHT-2) ) );
        bitBlt( &allTiles, 0, 0, &scalledBackground, 0, 0, scalledBackground.width(), scalledBackground.height() );
    }

    // handle default tiles
    bitBlt( &allTiles, 0, TILE_SIZE*(FIELD_HEIGHT-2),
            &tiles, 0, 0, tiles.width(), tiles.height() );

    // load tiles into canvas
    setTiles( allTiles, FIELD_WIDTH, FIELD_HEIGHT, TILE_SIZE, TILE_SIZE );
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

JezzGame::JezzGame( const QPixmap &background, int ballNum, QWidget *parent, const char *name )
    : QWidget( parent, name ), m_wall1( 0 ), m_wall2( 0 ),
      m_text( 0 ), m_running( false ), m_percent( 0 ), m_pictured( false )
{
   QString path = kapp->dirs()->findResourceDir( "data", "kbounce/pics/ball0000.png" ) + "kbounce/pics/";

   // load gfx
   m_ballPixmaps = new QCanvasPixmapArray( path + "ball%1.png", 25 );
   for ( unsigned n=0; n<m_ballPixmaps->count(); n++ )
       m_ballPixmaps->image(n)->setOffset( 0, 0 );
   QPixmap tiles( path + "tiles.png" );

   // setup arts
#if HAVE_ARTS
   m_artsServer = new SimpleSoundServer;
   *m_artsServer = Arts::Reference("global:Arts_SimpleSoundServer");
   if ( m_artsServer->isNull() )
       kdDebug(12008) << "Can't connect to aRts sound server" << endl;
#endif
   m_soundPath = kapp->dirs()->findResourceDir( "data", "kbounce/sounds/death.au" ) +
                 "kbounce/sounds/";

   // create field
   m_field = new JezzField( tiles, background, this, "m_field" );
   m_field->resize( TILE_SIZE*FIELD_WIDTH, TILE_SIZE*FIELD_HEIGHT );

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
      ball->setVelocity( ((kapp->random() & 1)*2-1)*2, ((kapp->random() & 1)*2-1)*2 );
      ball->setFrame( kapp->random() % 25 );
      ball->move( 4*TILE_SIZE + kapp->random() % ( (FIELD_WIDTH-8)*TILE_SIZE ),
                  4*TILE_SIZE + kapp->random() % ( (FIELD_HEIGHT-8)*TILE_SIZE ) );
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
#if HAVE_ARTS
    delete m_artsServer;
#endif
}


void JezzGame::display( const QString &text, int size )
{
    qDebug("This function \"display\" shouldn't be called!!!");
    if ( !text.isEmpty() )
    {
        //kdDebug(12008) << "text = " << text << endl;

        QFont font = KGlobalSettings::generalFont();
        font.setBold(true);
        font.setPointSize(size);
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

void JezzGame::playSound( const QString &name )
{
#if HAVE_ARTS
    if( !m_artsServer->isNull() && m_sound)
    {
        QString path = m_soundPath + name;
        m_artsServer->play( path.latin1() );
    }
#else
	return;
#endif
}

void JezzGame::setBackground( const QPixmap &background )
{
    m_field->setBackground( background );
}

void JezzGame::setSound( bool sound )
{
    m_sound = sound;
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
             m_field->setGameTile( x, y, true );
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

   return 100 * notFree / ( (FIELD_WIDTH-2) * (FIELD_HEIGHT-2) );
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
   if ( tile!=TILE_BORDER && tile>TILE_FREE && tile!=TILE_WALLEND )
   {
      kdDebug(12008) << "Collision" << endl;

      // play explosion sound
      playSound( "death.au" );

      // stop walls
      if ( (tile==TILE_WALLUP || tile==TILE_WALLLEFT) && m_wall1 )
      {
          kdDebug(12008) << "up or left" << endl;
          m_wall1->finish();
          m_wall1->fill( false );
          delete m_wall1;
          m_wall1 = 0;
      }

      if ( (tile==TILE_WALLDOWN || tile==TILE_WALLRIGHT) && m_wall2 )
      {
          kdDebug(12008) << "down or right" << endl;
          m_wall2->finish();
          m_wall2->fill( false );
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

   kdDebug(12008) << "JezzGame::buildWall( x=" << x << " y=" << y << " vertical=" << vertical << " )" << endl;
   if ( m_field->tile(x, y)==TILE_FREE )
   {
       playSound( "wallstart.au" );

      // check whether there is a ball at the moment
      QCanvasItemList cols = m_field->collisions( QRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE) );
      if ( cols.count()>0 )
      {
         kdDebug(12008) << "Direct collision" << endl;
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
    //kdDebug(12008) << "wallFinished" << endl;
    playSound( "wallend.au" );

    if ( tile==TILE_WALLEND )
    {
        if ( m_wall1 )
        {
            m_wall1->fill( false );
            delete m_wall1;
            m_wall1 = 0;
        }

        if ( m_wall2 )
        {
            m_wall2->fill( false );
            delete m_wall2;
            m_wall2 = 0;
        }
    } else
    {
        if ( m_wall1==wall && m_wall1 )
        {
            m_wall1->fill( true );
            delete m_wall1;
            m_wall1 = 0;
        }

        if ( m_wall2==wall && m_wall2 )
        {
            m_wall2->fill( true );
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
