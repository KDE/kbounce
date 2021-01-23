/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2007 Tomasz Boczkowski <tboczkowski@onet.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GAME_WIDGET_H
#define GAME_WIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>

#include <KgSound>
#include <KgDifficulty>

#include "board.h"
#include "renderer.h"

class KBounceGameWidget : public QGraphicsView
{
    Q_OBJECT

    public:
        enum State { BeforeFirstGame, Running, BetweenLevels, Paused, Suspended, GameOver };

        explicit KBounceGameWidget( QWidget* parent = nullptr );
        ~KBounceGameWidget() override;

        int level();
        int score();
        KBounceGameWidget::State state() const { return m_state; }
        KBounceRenderer* renderer() { return &m_renderer; }

        QSize minimumSizeHint() const override;

    public Q_SLOTS:
        void closeGame();
        void newGame();
        void setPaused( bool );
        void settingsChanged();
        void setSuspended( bool );
        void handleLevelChanged();

    Q_SIGNALS:
        void gameOver();
        void levelChanged( int level );
        void scoreChanged( int score );
        void filledChanged( int filled );
        void livesChanged( int lives );
        void timeChanged( int time );
        void stateChanged( KBounceGameWidget::State state );

    protected Q_SLOTS:
        void onFillChanged( int filled );
        void onLivesChanged( int lives );
        void onWallDied();
        void tick();

    protected:
        void resizeEvent( QResizeEvent* event ) override;
        void mouseReleaseEvent( QMouseEvent* event ) override;
        void focusOutEvent(QFocusEvent *event) override;
        void closeLevel();
        void newLevel();
        void updateCursor();
        void redraw();
        void setGameDifficulty(const KgDifficultyLevel *);

        KBounceRenderer m_renderer;

        QTimer* m_clock;
        KBounceBoard* m_board;
        State m_state;
        int m_bonus;
        int m_level;
        int m_score;
        int m_lives;
        int m_time;

        QGraphicsPixmapItem* m_overlay;
        void generateOverlay();

        bool m_vertical;
        QGraphicsScene m_scene;
        KgSound m_soundTimeout;
};

#endif
