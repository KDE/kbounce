/*
 * Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License,Life or (at your option) any later version.
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

#include <qlayout.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaction.h>
#include <kstdaction.h>
#include <qtimer.h>
#include <qlcdnumber.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kfiledialog.h>

#include "kjezz.h"
#include "game.h"
#include "newscoredialog.h"
#include "scoredialog.h"

KJezzball::KJezzball()
    : KMainWindow(0), m_gameWidget( 0 )
{
    // setup variables
    m_game.level = 1;
    m_game.score = 0;
    m_state = Idle;

    KConfig *config = kapp->config();
    m_backgroundDir = config->readPathEntry( "BackgroundDir" );
    m_showBackground = config->readBoolEntry( "ShowBackground", false );

    initGUI();

    // create widgets
    m_view = new QWidget( this, "m_view" );
    setCentralWidget( m_view );

    m_layout = new QGridLayout( m_view, 1, 3 );
    m_layout->setColStretch( 2, 1 );

    QVBoxLayout *infoLayout = new QVBoxLayout;
    m_layout->addLayout( infoLayout, 0, 1 );

    QLabel *label = new QLabel( i18n("Level:"), m_view );
    infoLayout->addWidget( label );
    m_levelLCD = new QLCDNumber( 5, m_view );
    infoLayout->addWidget( m_levelLCD );

    label = new QLabel( i18n("Score:"), m_view );
    infoLayout->addWidget( label );
    m_scoreLCD = new QLCDNumber( 5, m_view );
    infoLayout->addWidget( m_scoreLCD );

    infoLayout->addSpacing( 20 );

    label = new QLabel( i18n("Filled area:"), m_view );
    infoLayout->addWidget( label );
    m_percentLCD  = new QLCDNumber( 5, m_view );
    infoLayout->addWidget( m_percentLCD );

    label = new QLabel( i18n("Lives:"), m_view );
    infoLayout->addWidget( label );
    m_lifesLCD  = new QLCDNumber( 5, m_view );
    infoLayout->addWidget( m_lifesLCD );

    label = new QLabel( i18n("Time:"), m_view );
    infoLayout->addWidget( label );
    m_timeLCD  = new QLCDNumber( 5, m_view );
    infoLayout->addWidget( m_timeLCD );

    // create timers
    m_nextLevelTimer = new QTimer( this, "m_nextLevelTimer" );
    connect( m_nextLevelTimer, SIGNAL(timeout()), this, SLOT(switchLevel()) );

    m_gameOverTimer = new QTimer( this, "m_gameOverTimer" );
    connect( m_gameOverTimer, SIGNAL(timeout()), this, SLOT(gameOverNow()) );

    m_timer = new QTimer( this, "m_timer" );
    connect( m_timer, SIGNAL(timeout()), this, SLOT(second()) );

    // create demo game
    createLevel( 1 );
    m_gameWidget->display( i18n("Press CTRL-N to start a game!") );
}

KJezzball::~KJezzball()
{
}

void KJezzball::initGUI()
{
    KStdAction::openNew( this, SLOT(newGame()), actionCollection() );
    KStdAction::quit( kapp, SLOT(quit()), actionCollection() );
    (void)new KAction( i18n("S&how Highscore"), CTRL+Key_H, this, SLOT(showHighscore()),
                       actionCollection(), "file_highscore" );

    (void)new KAction( i18n("&Pause"), "player_pause", Key_P, this, SLOT(pauseGame()),
                       actionCollection(), "file_pause" );
    (void)new KAction( i18n("&Stop"), "player_stop", Key_S, this, SLOT(closeGame()),
                       actionCollection(), "file_stop" );

    (void)new KAction( i18n("&Select Images..."), 0, this, SLOT(selectBackground()),
                       actionCollection(), "background_select" );
    KToggleAction *show = new KToggleAction( i18n("Show &Images"), 0, this, SLOT(showBackground()),
                                             actionCollection(), "background_show" );

    show->setEnabled( !m_backgroundDir.isEmpty() );
    show->setChecked( m_showBackground );

    createGUI( "kjezzui.rc" );
}

void KJezzball::newGame()
{
    // Check for running game
    closeGame();
    if ( m_state==Idle )
    {
        // update displays
        m_game.level = 1;
        m_game.score = 0;

        m_levelLCD->display( m_game.level );
        m_scoreLCD->display( m_game.score );

        // start new game
        m_state = Running;

        createLevel( m_game.level );
        startLevel();
    }
}

void KJezzball::closeGame()
{
    if ( m_state!=Idle )
    {
        int ret = KMessageBox::questionYesNo( this, i18n("Do you really want to close the running game?") );
        if ( ret==KMessageBox::Yes )
        {
            stopLevel();
            m_state = Idle;
            highscore();
        }
    }
}

void KJezzball::pauseGame()
{
    switch ( m_state )
    {
    case Running:
        m_state = Paused;
        m_gameWidget->display( i18n("Game paused. Press P to continue!") );
        stopLevel();
        break;

    case Paused:
    case Suspend:
        m_state = Running;
        m_gameWidget->display( QString::null );
        startLevel();
        break;

    case Idle:
        break;
    }
}

void KJezzball::gameOver()
{
    stopLevel();
    m_gameOverTimer->start( 100, TRUE );
}

void KJezzball::gameOverNow()
{
    m_state = Idle;

    QString score;
    score.setNum( m_game.score );
    KMessageBox::information( this, i18n("Game Over!!! Score: %1").arg(score) );

    m_gameWidget->display( i18n("Game over. Press CTRL-N for a new game!") );

    highscore();
}

void KJezzball::showHighscore()
{
    ScoreDialog h(this);
    h.exec();
}

void KJezzball::selectBackground()
{
    QString path = KFileDialog::getExistingDirectory( m_backgroundDir,  this,
                                                      i18n("Select background image directory") );
    if ( !path.isEmpty() && path!=m_backgroundDir ) {
        m_backgroundDir = path;

        // enable action
        KAction *a = actionCollection()->action("background_show");
        if ( a ) a->setEnabled(true);

        // save settings
        KConfig *config = kapp->config();
        config->writeEntry( "BackgroundDir", m_backgroundDir );
        config->sync();

        // apply background setting
        if ( m_showBackground ) {
            if ( m_background.width()==0 )
                m_background = getBackgroundPixmap();

            m_gameWidget->setBackground( m_background );
        }
    }
}

void KJezzball::showBackground()
{
    KToggleAction *a = dynamic_cast<KToggleAction*>(actionCollection()->action("background_show"));
    if( a ) {

        bool show = a->isChecked();
        if ( show!=m_showBackground ) {

            m_showBackground = show;

            // save setting
            KConfig *config = kapp->config();
            config->writeEntry( "ShowBackground", m_showBackground );
            config->sync();

            // update field
            if ( m_showBackground ) {
                if ( m_background.width()==0 )
                    m_background = getBackgroundPixmap();
            }

            m_gameWidget->setBackground( m_showBackground ? m_background : QPixmap() );
        }

    }
}

QPixmap KJezzball::getBackgroundPixmap()
{
    // list directory
    QDir dir( m_backgroundDir, "*.png *.jpg", QDir::Name|QDir::IgnoreCase, QDir::Files );
    if ( !dir.exists() ) {
        kdDebug(1433) << "Directory not found" << endl;
        return QPixmap();
    }

    // return random pixmap
    int num = rand() % dir.count();
    return QPixmap( dir.absFilePath( dir[num] ) );
}

void KJezzball::focusOutEvent( QFocusEvent *ev )
{
    if ( m_state==Running )
    {
        stopLevel();
        m_state = Suspend;
        m_gameWidget->display( i18n("Game suspended") );
    }

    KMainWindow::focusOutEvent( ev );
}

void KJezzball::focusInEvent ( QFocusEvent *ev )
{
    if ( m_state==Suspend )
    {
        startLevel();
        m_state = Running;
        m_gameWidget->display( QString::null );
    }

    KMainWindow::focusInEvent( ev );
}

void KJezzball::second()
{
    m_level.time--;
    m_timeLCD->display( m_level.time );
    if ( m_level.time<=0 )
    {
        JezzGame::playSound( "timeout.au" );
        gameOver();
    } else
        if ( m_level.time<=30 )
            JezzGame::playSound( "seconds.au" );
}

void KJezzball::died()
{
    kdDebug() << "died" << endl;
    m_level.lifes--;
    m_lifesLCD->display( m_level.lifes );
    if ( m_level.lifes==0 ) gameOver();
}

void KJezzball::newPercent( int percent )
{
    m_percentLCD->display( percent );
    if ( percent>=75 )
    {
        m_level.score = m_level.lifes*15 + (percent-75)*2*(m_game.level+5);
        nextLevel();
    }
}

void KJezzball::createLevel( int level )
{
    // destroy old game
    if ( m_gameWidget ) delete m_gameWidget;

    // create new game widget
    if ( m_showBackground )
        m_background = getBackgroundPixmap();
    else
        m_background = QPixmap();

    m_gameWidget = new JezzGame( m_background, level+1, m_view, "m_gameWidget" );

    m_gameWidget->show();
    m_layout->addWidget( m_gameWidget, 0, 0 );
    connect( m_gameWidget, SIGNAL(died()), this, SLOT(died()) );
    connect( m_gameWidget, SIGNAL(newPercent(int)), this, SLOT(newPercent(int)) );

    // update displays
    m_level.lifes = level+1;
    m_lifesLCD->display( m_level.lifes );
    m_percentLCD->display( 0 );

    m_level.time = (level+2)*30;
    m_timeLCD->display( m_level.time );

    m_level.score = 0;
}

void KJezzball::startLevel()
{
    if ( m_gameWidget )
    {
        m_timer->start( 1000 );
        m_gameWidget->start();
    }
}

void KJezzball::stopLevel()
{
    if ( m_gameWidget )
    {
        m_gameWidget->stop();
        m_timer->stop();
    }
}

void KJezzball::nextLevel()
{
    stopLevel();
    m_nextLevelTimer->start( 100, TRUE );
}

void KJezzball::switchLevel()
{
    m_game.score += m_level.score;
    m_scoreLCD->display( m_game.score );

    QString score;
    score.setNum( m_level.score );

    QString level;
    level.setNum( m_game.level );

    KMessageBox::information( this, i18n("You've completed level %1 with "
        "a score of %2.\nGet ready for the next one!").arg(level).arg(score));

    m_game.level++;
    m_levelLCD->display( m_game.level );

    createLevel( m_game.level );
    startLevel();
}


void KJezzball::highscore()
{
    // Highscore?
    KConfig *config = kapp->config();
    config->setGroup("High Score");
    QString num;

    int i;
    for ( i=1; i<=10; ++i )
    {
        num.setNum( i );
        int score = config->readNumEntry( "Pos" + num + "Score", 0 );
        if ( m_game.score > score )
            break;
    }

    if (i <= 10)
    {
        // ask user for name
        NewScoreDialog d(this);
        if (d.exec())
        {
            // make place for new entry
            for (int j = 10; j > i; --j) {
                num.setNum( j - 1 );

                int level = config->readNumEntry( "Pos" + num + "Level", 0 );
                int score = config->readNumEntry( "Pos" + num + "Score", 0 );
                QString  name = config->readEntry( "Pos" + num + "Name", "Noname" );

                num.setNum( j );
                config->writeEntry( "Pos" + num + "Level", level );
                config->writeEntry( "Pos" + num + "Score", score );
                config->writeEntry( "Pos" + num + "Name", name );
            }

            // insert new entry
            num.setNum(i);
            config->writeEntry( "Pos" + num + "Level", m_game.level );
            config->writeEntry( "Pos" + num + "Score", m_game.score );
            config->writeEntry( "Pos" + num + "Name", d.name() );

            showHighscore();
        }
    }
}

#include "kjezz.moc"
