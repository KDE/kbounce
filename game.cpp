/*
 * Copyright (C) 2000-2005 Stefan Schimanski <1Stein@gmx.de>
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

#include <QTimer>
#include <QMouseEvent>
#include <QImage>
#include <QPainter>
#include <QFileInfo>
#include <QDir>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kurl.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <krandom.h>
#include <phonon/audioplayer.h>

#include "game.h"


#define TILE_SIZE 16

#define TILE_FIRST (0)
#define TILE_FREE (TILE_FIRST + 0)
#define TILE_BORDER (TILE_FIRST + 1)
#define TILE_WALL (TILE_FIRST + 2)
#define TILE_WALLEND (TILE_FIRST + 3)
#define TILE_WALLUP (TILE_FIRST + 4)
#define TILE_WALLDOWN (TILE_FIRST + 5)
#define TILE_WALLLEFT (TILE_FIRST + 6)
#define TILE_WALLRIGHT (TILE_FIRST + 7)

#define GAME_DELAY 15
#define BALL_ANIM_DELAY 60
#define WALL_DELAY 100


Phonon::AudioPlayer* JezzGame::m_player = 0;
QString JezzGame::m_soundPath;
bool JezzGame::m_sound = true;

#define MS2TICKS( ms ) ((ms)/GAME_DELAY)

Ball::Ball(const QList<QPixmap> &array, QGraphicsScene *scene)
    : QGraphicsPixmapItem( 0, scene ), m_animDelay( 0 ), m_soundDelay( MS2TICKS(BALL_ANIM_DELAY)/2 ),
    m_frames(array), m_currentFrame(0), m_xVelocity(0), m_yVelocity(0)
{
    setFrame(0);
}

void Ball::setFrame(int frame)
{
    if( !m_frames.isEmpty() )
    {
        m_currentFrame = frame % m_frames.count();
        setPixmap( m_frames.at( m_currentFrame ) );
    }
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
   if(stage == 0)
       return;
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
   QRect r = sceneBoundingRect().toRect();
   r.translate( static_cast<int>( xVelocity() ), static_cast<int>( yVelocity() ));
   JezzField* field = static_cast<JezzField*>(scene());

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

   setFrame(++m_currentFrame);
   moveBy( m_xVelocity, m_yVelocity );
   // update field
   update();
}

bool Ball::collide( double dx, double dy )
{
   QRect r = sceneBoundingRect().toRect();
   r.translate( static_cast<int>(dx), static_cast<int>(dy) );
   JezzField* field = static_cast<JezzField*>(scene());

   int ul = field->tile( r.left() / TILE_SIZE, r.top() / TILE_SIZE );
   int ur = field->tile( r.right() / TILE_SIZE, r.top() / TILE_SIZE );
   int bl = field->tile( r.left() / TILE_SIZE, r.bottom() / TILE_SIZE );
   int br = field->tile( r.right() / TILE_SIZE, r.bottom() / TILE_SIZE );

   return ( ul!=TILE_FREE || ur!=TILE_FREE || bl!=TILE_FREE || br!=TILE_FREE );
}

/*************************************************************************/

Wall::Wall( JezzField *field, int x, int y, Direction dir, int tile, QObject *parent )
    : QObject( parent ), m_dir( dir ), m_field( field ), m_startX( x ), m_startY( y ),
      m_tile( tile ), m_delay( MS2TICKS(WALL_DELAY)/2 ), m_active( true )
{
   //kDebug(12008) << "Wall::Wall" << endl;

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
        QList<QGraphicsItem*> cols = m_field->items( QRect(x*TILE_SIZE, y*TILE_SIZE,
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

TiledScene::TiledScene( QObject *parent )
    : QGraphicsScene(parent), m_bkgndTileNum(-1)
{

}

void TiledScene::setTile( int x, int y, int tilenum )
{
    m_tiles[y*m_numTilesH + x] = tilenum;
}

void TiledScene::setTiles( const QPixmap& p, const QPixmap& customBkgnd, int h, int v, int tilewidth, int tileheight )
{
    m_tilesPix = p;
    m_customBackground = customBkgnd;
    m_tilew = tilewidth;
    m_tileh = tileheight;
    m_numTilesH = h;
    m_numTilesV = v;
    for(int i=0;i<m_numTilesH*m_numTilesV;++i)
        m_tiles.append(0);
}

int TiledScene::tile( int x, int y ) const
{
    if(x >= m_numTilesH || x < 0 || y >= m_numTilesV || y < 0)
        return 0;
    return m_tiles.at( y*m_numTilesH + x );
}

void TiledScene::drawBackground( QPainter* p, const QRectF& rect )
{
    // FIXME dimsuz: 
    Q_UNUSED(rect);
    bool backgroundValid = !m_customBackground.isNull();
    for(int i=0;i<m_tiles.count();++i)
    {
        int tilenum = m_tiles.at(i);
        if(backgroundValid && (m_bkgndTileNum == tilenum))
        {
            // draw a tile from background (same pos)
            QRect srcRect( (i % m_numTilesH)*m_tilew, (i/m_numTilesH)*m_tileh, m_tilew, m_tileh );
            p->drawPixmap( srcRect.topLeft(), m_customBackground, srcRect );
        }
        else
        {
            // draw a tile from m_tilesPix
            QRect srcRect((tilenum % m_numTilesH)*m_tilew, (tilenum/m_numTilesH)*m_tileh, m_tilew, m_tileh);
            p->drawPixmap( QPoint((i % m_numTilesH)*m_tilew, (i/m_numTilesH)*m_tileh), m_tilesPix, srcRect );
        }
    }
}

/*************************************************************************/

JezzField::JezzField( const QPixmap &tiles, const QPixmap &background, QObject* parent )
    : TiledScene( parent ), m_tiles(tiles)
{
    setPixmaps( tiles, background );
}


void JezzField::setGameTile( int x, int y, bool black )
{
    if( m_background )
    {
        // this will ensure that upon redraw TILE_WALL tiles 
        // will be taken from background image and not from usual tileset
        setBackgroundTileNum( TILE_WALL );
    }
    setTile( x, y, black ? TILE_WALL : TILE_FREE );
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

            if ( tile<TILE_FIRST )
                tile = TILE_BORDER;

            setTile( x, y, tile );
        }

        setTile( FIELD_WIDTH-1, y, TILE_BORDER );
    }
    for ( int x=0; x<FIELD_WIDTH; x++ )
        setTile( x, FIELD_HEIGHT-1, TILE_BORDER );
}

void JezzField::setPixmaps( const QPixmap &tiles, const QPixmap &background )
{
    QPixmap bkgnd;

    if ( background.width()==0 || background.height()==0 ) {
        m_background = false;
    } else {
        // handle background
        m_background = true;
        QImage img = background.toImage();
        QPixmap scalledBackground = QPixmap::fromImage(
                                   img.scaled( TILE_SIZE*(FIELD_WIDTH-2),
                                               TILE_SIZE*(FIELD_HEIGHT-2),
                                               Qt::IgnoreAspectRatio,
                                               Qt::SmoothTransformation ) );
        bkgnd = QPixmap( TILE_SIZE*FIELD_WIDTH, TILE_SIZE*FIELD_HEIGHT );
        QPainter p(&bkgnd);
        p.drawPixmap( TILE_SIZE, TILE_SIZE, scalledBackground );
    }
    setTiles( tiles, bkgnd, FIELD_WIDTH, FIELD_HEIGHT, TILE_SIZE, TILE_SIZE );
}


/*************************************************************************/

JezzView::JezzView(QGraphicsScene* viewing, QWidget* parent)
   : QGraphicsView( viewing, parent ), m_vertical( false )
{
   //setResizePolicy( AutoOne );
   setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   setCacheMode( QGraphicsView::CacheBackground );

   setCursor( Qt::SizeHorCursor );
}

void JezzView::mouseReleaseEvent( QMouseEvent *ev )
{
   if ( ev->button() & Qt::RightButton )
   {
      m_vertical = !m_vertical;
      setCursor( m_vertical ? Qt::SizeVerCursor : Qt::SizeHorCursor );
   }

   if ( ev->button() & Qt::LeftButton )
   {
      emit buildWall( ev->x()/TILE_SIZE, ev->y()/TILE_SIZE, m_vertical );
   }
}

/*************************************************************************/

JezzGame::JezzGame( const QPixmap &background, int ballNum, QWidget *parent )
    : QWidget( parent ), m_wall1( 0 ), m_wall2( 0 ),
      m_text( 0 ), m_running( false ), m_percent( 0 ), m_pictured( false )
{
   QString path = kapp->dirs()->findResourceDir( "data", "kbounce/pics/ball0000.png" ) + "kbounce/pics/";

   // load gfx
   QFileInfo fi(path + "ball*.png");
   m_ballPixmaps = new QList<QPixmap>;
   foreach( const QString &entry, QDir(fi.path(), fi.fileName()).entryList() )
       m_ballPixmaps->append( fi.path() + '/' + entry );
   QPixmap tiles( path + "tiles.png" );

   // setup audio player
   m_player = new Phonon::AudioPlayer(Phonon::GameCategory);
   m_soundPath = kapp->dirs()->findResourceDir( "data", "kbounce/sounds/death.au" ) +
                 "kbounce/sounds/";

   // create field
   m_field = new JezzField( tiles, background, this );
   m_field->setSceneRect( 0, 0, TILE_SIZE*FIELD_WIDTH, TILE_SIZE*FIELD_HEIGHT );

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
   m_view = new JezzView( m_field, this );
   m_view->move( 0, 0 );
   m_view->adjustSize();
   connect( m_view, SIGNAL(buildWall(int, int, bool)), this, SLOT(buildWall(int, int, bool)) );

   // create balls
   for ( int n=0; n<ballNum; n++ )
   {
      Ball *ball = new Ball( *m_ballPixmaps, m_field );
      m_balls.append( ball );
      ball->setVelocity( ((KRandom::random() & 1)*2-1)*2, ((KRandom::random() & 1)*2-1)*2 );
      ball->setFrame( KRandom::random() % 25 );
      ball->setPos( 4*TILE_SIZE + KRandom::random() % ( (FIELD_WIDTH-8)*TILE_SIZE ),
                  4*TILE_SIZE + KRandom::random() % ( (FIELD_HEIGHT-8)*TILE_SIZE ) );
      ball->show();
   }

   // create text label
   m_text = new QGraphicsSimpleTextItem( 0, m_field );

   // create game clock
   m_clock = new QTimer( this );
   connect( m_clock, SIGNAL(timeout()), this, SLOT(tick()) );
   m_clock->start( GAME_DELAY );

   // setup geometry
   setFixedSize( m_view->size() );
}

JezzGame::~JezzGame()
{
    qDeleteAll( m_balls );
    m_balls.clear();
    delete m_view;
    delete m_field;
    delete m_ballPixmaps;
    delete m_player;
}


void JezzGame::display( const QString &text, int size )
{
    kDebug(12008) << "This function \"display\" shouldn't be called!!!" << endl;
    if ( !text.isEmpty() )
    {
        //kDebug(12008) << "text = " << text << endl;

        QFont font = KGlobalSettings::generalFont();
        font.setBold(true);
        font.setPointSize(size);
        m_text->setFont( font );
        m_text->setText( text );

        QRect size = m_text->boundingRect().toRect();
        m_text->setPos( ( FIELD_WIDTH*TILE_SIZE - size.width() ) / 2,
                      ( FIELD_HEIGHT*TILE_SIZE - size.height() ) / 2 );

        m_text->show();
    } else
    {
        m_text->hide();
    }
}

void JezzGame::playSound( const QString &name )
{
#ifdef __GNUC__
#warning reenable me after phonon is set up
#endif
    Q_UNUSED(name);
    /* FIXME: TEMPORARILY COMMENTED OUT to prevent error mboxes from phonon
    if( m_player && m_sound)
        m_player->play( KUrl::fromPath(m_soundPath + name) );
         */
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
   foreach( Ball *ball, m_balls )
      fill( static_cast<int>( ball->x()/TILE_SIZE ),
            static_cast<int>( ball->y()/TILE_SIZE ) );

   // areas still free can be blacked now
   for ( int y=1; y<FIELD_HEIGHT-1; y++ )
      for ( int x=1; x<FIELD_WIDTH-1; x++ )
      {
         if ( m_buf[x][y]==TILE_FREE )
             m_field->setGameTile( x, y, true );
      }

   m_field->update();
   m_view->update();

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
      m_buf[_x][y] = TILE_WALL;
   int stopx = _x;

   // fill above
   for ( _x=x; _x>stopx; _x-- )
      if ( m_buf[_x][y-1]==TILE_FREE ) fill( _x, y-1 );

   // fill below
   for ( _x=x; _x>stopx; _x-- )
      if ( m_buf[_x][y+1]==TILE_FREE ) fill( _x, y+1 );

   // go right
   for ( _x=x+1; m_buf[_x][y]==TILE_FREE; _x++ )
      m_buf[_x][y] = TILE_WALL;
   stopx = _x;

   // fill above
   for ( _x=x+1; _x<stopx; _x++ )
      if ( m_buf[_x][y-1]==TILE_FREE ) fill( _x, y-1 );

   // fill below;
   for ( _x=x+1; _x<stopx; _x++ )
      if ( m_buf[_x][y+1]==TILE_FREE ) fill( _x, y+1 );
}

void JezzGame::ballCollision( Ball * /*ball*/, int /*x*/, int /*y*/, int tile )
{
   if ( tile!=TILE_BORDER && tile != TILE_WALL && tile>TILE_FREE && tile!=TILE_WALLEND )
   {
      kDebug(12008) << "Collision" << endl;

      // play explosion sound
      playSound( "death.au" );

      // stop walls
      if ( (tile==TILE_WALLUP || tile==TILE_WALLLEFT) && m_wall1 )
      {
          kDebug(12008) << "up or left" << endl;
          m_wall1->finish();
          m_wall1->fill( false );
          delete m_wall1;
          m_wall1 = 0;
      }

      if ( (tile==TILE_WALLDOWN || tile==TILE_WALLRIGHT) && m_wall2 )
      {
          kDebug(12008) << "down or right" << endl;
          m_wall2->finish();
          m_wall2->fill( false );
          delete m_wall2;
          m_wall2 = 0;
      }

      m_view->resetCachedContent(); //redraw background
      // update view
      m_field->update();
      m_view->update();

      // send death msg
      emit died();
   }
}

void JezzGame::buildWall( int x, int y, bool vertical )
{
    if ( !m_running ) return;

   kDebug(12008) << "JezzGame::buildWall( x=" << x << " y=" << y << " vertical=" << vertical << " )" << endl;
   if ( m_field->tile(x, y)==TILE_FREE )
   {
       playSound( "wallstart.au" );

      // check whether there is a ball at the moment
      QList<QGraphicsItem*> cols = m_field->items( QRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE) );
      if ( cols.count()>0 )
      {
         kDebug(12008) << "Direct collision" << endl;
         emit ballCollision( (Ball*)cols.first(), x, y, TILE_WALLUP );
         return;
      }

      // start walls
      if ( !m_wall1 )
      {
         m_wall1 = new Wall( m_field, x, y,
                             vertical? Wall::Up : Wall::Left,
                             vertical? TILE_WALLUP : TILE_WALLLEFT,
                             this );
         connect( m_wall1, SIGNAL(finished(Wall *, int)),
                  this, SLOT(wallFinished(Wall *, int)) );            }

      if ( !m_wall2 )
      {
         m_wall2 = new Wall( m_field, x, y,
                             vertical? Wall::Down: Wall::Right,
                             vertical? TILE_WALLDOWN : TILE_WALLRIGHT,
                             this );
         connect( m_wall2, SIGNAL(finished(Wall *, int)),
                  this, SLOT(wallFinished(Wall *, int)) );
      }
   }
}

void JezzGame::wallFinished( Wall *wall, int tile )
{
    //kDebug(12008) << "wallFinished" << endl;
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

    makeBlack();
    m_view->resetCachedContent(); //redraw background
}

void JezzGame::tick()
{
    if ( m_running )
    {
        if ( m_field ) m_field->advance();
        if ( m_wall1 ) m_wall1->advance();
        if ( m_wall2 ) m_wall2->advance();
        if( m_wall1 || m_wall2 )
            m_view->resetCachedContent(); //redraw background
    } else
    {
        foreach( Ball *ball, m_balls )
            ball->update();

        if ( m_field ) m_field->update();
        if ( m_wall1 ) m_wall1->update();
        if ( m_wall2 ) m_wall2->update();
        if( m_wall1 || m_wall2 )
            m_view->resetCachedContent(); //redraw background
    }
}

#include "game.moc"
