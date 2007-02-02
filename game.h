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

#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QList>

class QTimer;
class JezzField;

namespace Phonon
{
    class AudioPlayer;
}

#define FIELD_WIDTH 32
#define FIELD_HEIGHT 20

class Ball : public QGraphicsPixmapItem
{
 public:
    Ball(const QList<QPixmap> &frames, QGraphicsScene* scene);

    void update();
    void advance(int stage);
    bool collide( double dx=0, double dy=0 );

    void setFrame(int frame);
    int frame() const { return m_currentFrame; }
    int frameCount() const { return m_frames.count(); }
    void setVelocity(qreal xv, qreal yv) { m_xVelocity = xv; m_yVelocity = yv; }
    void setXVelocity(qreal xv) { m_xVelocity = xv; }
    void setYVelocity(qreal yv) { m_yVelocity = yv; }
    qreal xVelocity() const { return m_xVelocity; }
    qreal yVelocity() const { return m_yVelocity; }
 protected:
    int m_animDelay;
    int m_soundDelay;
    QList<QPixmap> m_frames;
    int m_currentFrame;
    qreal m_xVelocity;
    qreal m_yVelocity;
};


class Wall : public QObject
{
   Q_OBJECT
public:
   enum Direction { Up, Down, Left, Right };

   Wall( JezzField *field, int x, int y, Direction dir, int tile,
	 QObject *parent=0 );

   void finish();
   void fill( bool black );

signals:
   void finished( Wall *wall, int tile );

public slots:
   void advance();
   void update();

private:
   bool isFree( int x, int y );

   Direction m_dir;
   JezzField *m_field;
   int m_dx, m_dy;
   int m_x, m_y;
   int m_startX, m_startY;
   int m_tile;
   int m_delay;
   bool m_active;
};

/**
 *  Convenience class to provide a tile functionality that Q3Canvas had
 */
class TiledScene : public QGraphicsScene
{
public:
    explicit TiledScene( QObject* parent = 0 );
    /**
     *  @see Q3Canvas::setTile
     */
    void setTile( int x, int y, int tilenum );
    /**
     *  Sets tile number which should be replaced by the tile from the customBkgnd rather
     *  then from m_tilesPix.
     */
    void setBackgroundTileNum( int num ) { m_bkgndTileNum = num; }
    /**
     *  @see Q3Canvas::setTiles
     */
    void setTiles( const QPixmap& p, const QPixmap& customBkgnd, int h, int v, int tilewidth, int tileheight );
    /**
     *  @see Q3Canvas::tile
     */
    int  tile( int x, int y ) const;
protected:
    virtual void drawBackground( QPainter*, const QRectF& ); // reimp
private:
    /**
     *  Pixmap which serves as origin for tiles
     */
    QPixmap m_tilesPix;
    /**
     *  Custom background (@see setBkgndTileNum)
     */
    QPixmap m_customBackground;
    /**
     *  Specifies that if the cell contains tile number m_bkgndTileNum then it should
     *  be painted with tile from customBkgnd (@see setTiles)
     *  if customBkgnd wasn't set this has no effect
     */
    int m_bkgndTileNum;
    /**
     *  Holds the tiles numbers
     *  @see Q3Canvas::setTiles
     */
    QList<int> m_tiles;
    /**
     *  Width of single tile
     */
    int m_tilew;
    /**
     *  Height of single tile
     */
    int m_tileh;
    /**
     *  Number of tiles in horizontal direction
     */
    int m_numTilesH;
    /**
     *  Number of tiles in vertical direction
     */
    int m_numTilesV;
};

class JezzField : public TiledScene
{
   Q_OBJECT
public:
   JezzField( const QPixmap &tiles, const QPixmap &background, QObject* parent = 0 );


   void setGameTile( int x, int y, bool black );
   void setBackground( const QPixmap &background );

signals:
   void ballCollision( Ball *ball, int x, int y, int tile );

private:
   bool m_background;
   QPixmap m_tiles;

   void setPixmaps( const QPixmap &tiles, const QPixmap &background );
   friend class Ball;
   void emitBallCollisiton( Ball *ball, int x, int y, int tile )
      { emit ballCollision( ball, x, y, tile ); };

};


class JezzView : public QGraphicsView
{
  Q_OBJECT
public:
   explicit JezzView(QGraphicsScene* viewing=0, QWidget* parent=0);

signals:
   void buildWall( int x, int y, bool vertical );

protected:
   void mouseReleaseEvent( QMouseEvent * );

private:
   bool m_vertical;
};


class JezzGame : public QWidget
{
   Q_OBJECT

public:
   JezzGame( const QPixmap &background, int ballNum, QWidget *parent=0 );
   ~JezzGame();

   int percent();
   static void playSound( const QString &name );
   void display( const QString &text, int size=20 );
   void setBackground( const QPixmap &background );

signals:
   void died();
   void newPercent( int percent );

public slots:
   void start();
   void stop();
   void setSound( bool sound );

protected slots:
   void tick();
   void buildWall( int x, int y, bool vertical );
   void wallFinished( Wall *wall, int tile );
   void ballCollision( Ball *ball, int x, int y, int tile );

protected:
   void makeBlack();
   void fill( int x, int y );
   void fillLeft( int x, int y );
   int m_buf[FIELD_WIDTH][FIELD_HEIGHT];

   JezzField *m_field;
   JezzView *m_view;

   Wall *m_wall1, *m_wall2;

   QList<Ball*> m_balls;
   QList<QPixmap> *m_ballPixmaps;
   QGraphicsSimpleTextItem *m_text;

   QTimer *m_clock;
   bool m_running;
   int m_percent;
   bool m_pictured;

   static Phonon::AudioPlayer* m_player;
   static QString m_soundPath;
   static bool m_sound;
};

#endif
