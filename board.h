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

#ifndef BOARD_H
#define BOARD_H

#include "kgamecanvas.h"

#include <QList>
#include <QSize>

#include "gameobject.h"
#include "renderer.h"

#define TILE_NUM_H 20
#define TILE_NUM_W 32

namespace Phonon
{
    class MediaObject;
}

class KBounceBall;
class KBounceWall;

class KBounceBoard: public QObject, public KGameCanvasGroup
{
    Q_OBJECT

    public:
	enum TileType{ Empty, Free, Border, Wall, Temp };

	explicit KBounceBoard( KBounceRenderer* renderer, KGameCanvasAbstract* canvas = NULL, QWidget* parent = 0 );
	~KBounceBoard();

	void resize( QSize& size );
	void redraw();

	void newLevel( int level );
	void setPaused( bool );

	void buildWall( const QPoint& pos, bool vertical );

	int balls();
	int filled();

	KBounceCollision checkCollision( void* object, const QRectF& rect, int type );
	KBounceCollision checkCollisionTiles( const QRectF& rect );
	void checkCollisions();

	QPoint mapPosition( const QPointF& pos ) const;
	QPointF unmapPosition( const QPoint& pos ) const;

	void playSound( const QString& name );
	void setSoundPath( const QString& path );
	void setSounds( bool val );

    signals:
	void ballsChanged( int balls );
	void fillChanged( int fill );
	void wallDied();

    protected slots:
	void tick();
        void wallFinished( int x1, int y1, int x2, int y2 );

    private:
	void clear();
	void fill( int x, int y );

	KBounceRenderer* m_renderer;

	TileType m_tiles[TILE_NUM_W][TILE_NUM_H];
	KGameCanvasPixmap* m_tilesPix;
	QSize m_tileSize;
	int m_filled;

	QList<KBounceBall*> m_balls;
	QList<KBounceWall*> m_walls;

	QTimer* m_clock;

	Phonon::MediaObject* m_audioPlayer;
	bool m_playSounds;
	QString m_soundPath;
};

#endif // BOARD_H

