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

#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <qwidget.h>
#include <qcanvas.h>
#include <arts/soundserver.h>

using namespace Arts;

class QTimer;
class JezzField;

#define FIELD_WIDTH 32
#define FIELD_HEIGHT 20

class Ball : public QCanvasSprite
{
 public:
    Ball(QCanvasPixmapArray* array, QCanvas* canvas);

    void update();
    void advance(int stage);
    bool collide( double dx=0, double dy=0 );

 protected:
    int m_animDelay;
    int m_soundDelay;
};


class Wall : public QObject
{
   Q_OBJECT
public:
   enum Direction { Up, Down, Left, Right };

   Wall( JezzField *field, int x, int y, Direction dir, int tile,
	 QObject *parent=0, const char *name=0 );

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


class JezzField : public QCanvas
{
   Q_OBJECT
public:
   JezzField( QPixmap tiles, QPixmap background, QObject* parent = 0, const char* name = 0 );

   void setGameTile( int x, int y, bool black );
   void setBackground( QPixmap background );

signals:
   void ballCollision( Ball *ball, int x, int y, int tile );

private:
   friend Ball;
   bool m_background;
   QPixmap m_tiles;
   QArray<QPixmap> m_backTiles;

   void setPixmaps( QPixmap tiles, QPixmap background );
   void emitBallCollisiton( Ball *ball, int x, int y, int tile )
      { emit ballCollision( ball, x, y, tile ); };

};


class JezzView : public QCanvasView
{
  Q_OBJECT
public:
   JezzView(QCanvas* viewing=0, QWidget* parent=0, const char* name=0, WFlags f=0);

signals:
   void buildWall( int x, int y, bool vertical );

protected:
   void viewportMouseReleaseEvent( QMouseEvent * );

private:
   bool m_vertical;
};


class JezzGame : public QWidget
{
   Q_OBJECT

public:
   JezzGame( QPixmap background, int ballNum, QWidget *parent=0, const char *name=0 );
   ~JezzGame();

   int percent();
   static void playSound( QString name );
   void display( QString text, int size=20 );
   void setBackground( QPixmap background );

signals:
   void died();
   void newPercent( int percent );

public slots:
   void start();
   void stop();

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

   QList<Ball> m_balls;
   QCanvasPixmapArray *m_ballPixmaps;
   QCanvasText *m_text;

   QTimer *m_clock;
   bool m_running;
   int m_percent;
   bool m_pictured;

   static SimpleSoundServer *m_artsServer;
   static QString m_soundPath;
};

#endif
