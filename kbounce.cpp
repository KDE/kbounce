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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qlayout.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaction.h>
#include <kstdgameaction.h>
#include <qtimer.h>
#include <qlcdnumber.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kstatusbar.h>
#include <khighscore.h>
#include <kexthighscore.h>

#include "kbounce.h"
#include "game.h"

KJezzball::KJezzball()
    : m_gameWidget( 0 )
{
    // setup variables
    m_game.level = 1;
    m_game.score = 0;
    m_state = Idle;

    KConfig *config = kapp->config();
    m_backgroundDir = config->readPathEntry( "BackgroundDir" );
    m_showBackground = config->readBoolEntry( "ShowBackground", false );

    statusBar();
    initXMLUI();

    m_soundAction -> setChecked((config->readBoolEntry( "PlaySounds", true )));

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
    statusBar()->message( i18n("Press %1 to start a game!")
                          .arg(m_newAction->shortcut().toString()) );
    //m_gameWidget->display( i18n("Press <Space> to start a game!") );

    setFocusPolicy(QWidget::StrongFocus);
    setFocus();
    setupGUI();
}

KJezzball::~KJezzball()
{
    KConfig *config = kapp->config();
    config->writeEntry( "PlaySounds", m_soundAction->isChecked() );
}

/**
 * create the action events create the gui.
 */
void KJezzball::initXMLUI()
{
    m_newAction = KStdGameAction::gameNew( this, SLOT(newGame()), actionCollection() );
    // AB: originally KBounce/KJezzball used Space for new game - but Ctrl+N is
    // default. We solve this by providing space as an alternative key
    KShortcut s = m_newAction->shortcut();
    s.append(KKeySequence(QKeySequence(Key_Space)));
    m_newAction->setShortcut(s);

    KStdGameAction::quit(this, SLOT(close()), actionCollection() );
    KStdGameAction::highscores(this, SLOT(showHighscore()), actionCollection() );
    m_pauseButton = KStdGameAction::pause(this, SLOT(pauseGame()), actionCollection());
    KStdGameAction::end(this, SLOT(closeGame()), actionCollection());
    KStdGameAction::configureHighscores(this, SLOT(configureHighscores()),actionCollection());

    new KAction( i18n("&Select Background Folder..."), 0, this, SLOT(selectBackground()),
                       actionCollection(), "background_select" );
    m_backgroundShowAction =
        new KToggleAction( i18n("Show &Backgrounds"), 0, this, SLOT(showBackground()),
                           actionCollection(), "background_show" );
    m_backgroundShowAction->setCheckedState(i18n("Hide &Backgrounds"));
    m_backgroundShowAction->setEnabled( !m_backgroundDir.isEmpty() );
    m_backgroundShowAction->setChecked( m_showBackground );

    m_soundAction = new KToggleAction( i18n("&Play Sounds"), 0, 0, 0, actionCollection(), "toggle_sound");
}

void KJezzball::newGame()
{
    // Check for running game
    closeGame();
    if ( m_state==Idle )
    {
        // untoggles the pause button in case it was toggled
        m_pauseButton->setChecked(false);

        // update displays
        m_game.level = 1;
        m_game.score = 0;

        m_levelLCD->display( m_game.level );
        m_scoreLCD->display( m_game.score );

        statusBar()->clear();

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
        int old_state = m_state;
        if (old_state == Running)
            pauseGame();
        int ret = KMessageBox::questionYesNo( this, i18n("Do you really want to close the running game?"), QString::null, KStdGuiItem::close(), KStdGuiItem::cancel() );
        if ( ret==KMessageBox::Yes )
        {
            stopLevel();
            m_state = Idle;
            highscore();
        }
        else if (old_state == Running)
        {
            pauseGame(); // Unpause
        }
    }
}

void KJezzball::pauseGame()
{
    switch ( m_state )
    {
    case Running:
        m_state = Paused;
        statusBar()->message(i18n("Game paused.") );
        //m_gameWidget->display( i18n("Game paused. Press P to continue!") );
        stopLevel();
        break;

    case Paused:
    case Suspend:
        m_state = Running;
        statusBar()->clear();
        //m_gameWidget->display( QString::null );
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
    KMessageBox::information( this, i18n("Game Over! Score: %1").arg(score) );
    statusBar()->message(  i18n("Game over. Press <Space> for a new game") );
    //m_gameWidget->display( i18n("Game over. Press <Space> for a new game!") );
    highscore();
}

/**
 * Bring up the standard kde high score configure dialog.
 */
void KJezzball::configureHighscores()
{
	KExtHighscore::configure(this);
}

/**
 * Bring up the standard kde high score dialog.
 */
void KJezzball::showHighscore()
{
    KExtHighscore::show(this);
}

/**
 * Select a background image.
 */
void KJezzball::selectBackground()
{
    QString path = KFileDialog::getExistingDirectory( m_backgroundDir,  this,
                                                      i18n("Select Background Image Folder") );
    if ( !path.isEmpty() && path!=m_backgroundDir ) {
        m_backgroundDir = path;

        // enable action
        m_backgroundShowAction->setEnabled(true);

        // save settings
        KConfig *config = kapp->config();
        config->writePathEntry( "BackgroundDir", m_backgroundDir );
        config->sync();

        // apply background setting
        if ( m_showBackground ) {
            if ( m_background.width()==0 )
                m_background = getBackgroundPixmap();

            m_gameWidget->setBackground( m_background );
        }
        else{
          KMessageBox::information( this, i18n("You may now turn on background images."));
        }
    }
}

void KJezzball::showBackground()
{
    bool show = m_backgroundShowAction->isChecked();
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

QPixmap KJezzball::getBackgroundPixmap()
{
    // list directory
    QDir dir( m_backgroundDir, "*.png *.jpg", QDir::Name|QDir::IgnoreCase, QDir::Files );
    if ( !dir.exists() ) {
        kdDebug(12008) << "Directory not found" << endl;
        return QPixmap();
    }

    if (dir.count() > 1)
    {
        // return random pixmap
        int num = kapp->random() % dir.count();
        return QPixmap( dir.absFilePath( dir[num] ) );
    }
    else if (dir.count()==1)
    {
    	return QPixmap( dir.absFilePath(dir[0]) );
    }
    else return QPixmap();
}

void KJezzball::focusOutEvent( QFocusEvent *ev )
{
    if ( m_state==Running )
    {
        stopLevel();
        m_state = Suspend;
        m_pauseButton->setChecked(true);
        statusBar()->message( i18n("Game suspended") );
        // m_gameWidget->display( i18n("Game suspended") );
    }

    KMainWindow::focusOutEvent( ev );
}

void KJezzball::focusInEvent ( QFocusEvent *ev )
{
    if ( m_state==Suspend )
    {
        startLevel();
        m_state = Running;
        statusBar()->clear();
        m_pauseButton->setChecked(false);
        //m_gameWidget->display( QString::null );
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
    kdDebug(12008) << "died" << endl;
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
    m_gameWidget->setSound(m_soundAction->isChecked());

    m_gameWidget->show();
    m_layout->addWidget( m_gameWidget, 0, 0 );
    connect( m_gameWidget, SIGNAL(died()), this, SLOT(died()) );
    connect( m_gameWidget, SIGNAL(newPercent(int)), this, SLOT(newPercent(int)) );
    connect( m_soundAction, SIGNAL(toggled(bool)), m_gameWidget, SLOT(setSound(bool)) );

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

    // make sure the LCD provides enough digits for the score
    // (fixes #96841)
    int numDigits=0;
    int temp_score = m_game.score;
    for ( ; temp_score > 0; ++numDigits ) temp_score /= 10;
    if ( numDigits < 5 ) numDigits = 5; // set numDigits to at least 5, otherwise it does not look well

    m_scoreLCD->setNumDigits( numDigits );
    m_scoreLCD->display( m_game.score );

    QString score;
    score.setNum( m_level.score );

    QString level;
    level.setNum( m_game.level );

QString foo = QString(
i18n("You have successfully cleared more than 75% of the board.\n") +
i18n("%1 points: 15 points per remaining life\n").arg(m_level.lifes*15) +
i18n("%1 points: Bonus\n").arg((m_gameWidget->percent()-75)*2*(m_game.level+5)) +
i18n("%1 points: Total score for this level\n").arg(score) +
i18n("On to level %1. Remember you get %2 lives this time!")).arg(m_game.level+1).arg(m_game.level+2);

   KMessageBox::information( this,foo );


   // KMessageBox::information( this, i18n("You've completed level %1 with "
   //     "a score of %2.\nGet ready for the next one!").arg(level).arg(score));

    m_game.level++;
    m_levelLCD->display( m_game.level );

    createLevel( m_game.level );
    startLevel();
}


void KJezzball::highscore()
{
    KExtHighscore::Score score(KExtHighscore::Won);
    score.setScore(m_game.score);
    score.setData("level", m_game.level);
    KExtHighscore::submitScore(score, this);
}

#include "kbounce.moc"
