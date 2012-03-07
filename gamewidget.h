/*
 * Copyright (C) 2000-2005 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2007 Tomasz Boczkowski <tboczkowski@onet.pl>
 *
 * This file is part of the KDE project "KBounce"
 *
 * KBounce is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * KBounce is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with KBounce; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef GAME_WIDGET_H
#define GAME_WIDGET_H

#include <kgamecanvas.h>
#include <QMouseEvent>

#include "board.h"
#include "renderer.h"

class KGameTheme;

class KBounceGameWidget : public KGameCanvasWidget
{
	Q_OBJECT

	public:
	enum State { BeforeFirstGame, Running, BetweenLevels, Paused, Suspended, GameOver };

	explicit KBounceGameWidget( QWidget* parent = 0 );
	~KBounceGameWidget();

	int level();
	int score();
	KBounceGameWidget::State state() const { return m_state; }

	QSize minimumSizeHint() const;

    public slots:
	void closeGame();
	void newGame();
	void setPaused( bool );
	void settingsChanged();
	void setSounds( bool );
	void setSoundPath( const QString& path );
	void setSuspended( bool );
	void levelChanged();

    signals:
	void gameOver();
	void levelChanged( int level );
	void scoreChanged( int score );
	void filledChanged( int filled );
	void livesChanged( int lives );
	void timeChanged( int time );
	void stateChanged( KBounceGameWidget::State state );

    protected slots:
	void onFillChanged( int filled );
	void onLivesChanged( int lives );
	void onWallDied();
	void tick();
	
    protected:
	virtual void resizeEvent( QResizeEvent* event );
	virtual void mouseReleaseEvent( QMouseEvent* event );
	void renderBackground();
	void closeLevel();
	void newLevel();

	void redraw();

	KBounceRenderer m_renderer;
	KGameTheme *m_theme;

	QTimer* m_clock;
	KBounceBoard* m_board;
	State m_state;
	int m_bonus;
	int m_level;
	int m_score;
	int m_lives;
	int m_time;

	KGameCanvasPixmap* m_overlay;
	void generateOverlay();

	bool m_vertical;
};

#endif

