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

class QTimer;
class JezzField;

#define FIELD_WIDTH 40
#define FIELD_HEIGHT 30

class Ball : public QCanvasSprite
{
public:
   Ball(QCanvasPixmapArray* array, QCanvas* canvas);

   void advance(int stage);

private:
   bool collide( double dx, double dy );
};


class Wall : public QObject
{
   Q_OBJECT
public:
   enum Direction { Up, Down, Left, Right };

   Wall( JezzField *field, int x, int y, Direction dir, QObject *parent=0, const char *name=0 );   

   void finish();
   void black();

signals:
   void finished( Wall *wall );

private slots:
   void update();

private:
   QTimer *m_timer;
   Direction m_dir;
   JezzField *m_field;
   int m_dx, m_dy;
   int m_x, m_y;
   int m_startX, m_startY;
   int m_tile;
};


class JezzField : public QCanvas
{
   Q_OBJECT
public:
   JezzField( QObject* parent = 0, const char* name = 0 );

signals:
   void ballCollision( Ball *ball, int x, int y, int tile );

private:
   friend Ball;

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
   JezzGame( int ballNum, QWidget *parent=0, char *name=0 );
   ~JezzGame();

   void makeBlack();

public slots:
   void tick();
   void buildWall( int x, int y, bool vertical );
   void wallFinished( Wall *wall );

protected:
   void fill( int x, int y );
   void fillLeft( int x, int y );
   char m_buf[FIELD_WIDTH][FIELD_HEIGHT];
   
   JezzField *m_field;
   JezzView *m_view;

   Wall *m_wall1, *m_wall2;

   QList<Ball> m_balls;
   QCanvasPixmapArray *m_ballPixmaps;

   QTimer *m_clock;

   int m_blackTiles;
};

#endif
