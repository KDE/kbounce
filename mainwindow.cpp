/*
 * Copyright (C) 2000-2005 Stefan Schimanski <1Stein@gmx.de>
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


#include "mainwindow.h"
#include "gamewidget.h"
#include "settings.h"
#include "backgroundselector.h"
#include "debug.h"

#include <KRandom>
#include <KStandardGameAction>
#include <KToggleAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KMessageBox>
#include <KScoreDialog>
#include <KgThemeSelector>
#include <KgDifficulty>
#include <KLocalizedString>

#include <QStatusBar>
#include <QAction>

KBounceMainWindow::KBounceMainWindow()
{
    //setComponentName(QStringLiteral("kbounce"), i18n("KBounce"));
    m_statusBar = statusBar();
    
    levelLabel->setText(i18n("Level: %1", QStringLiteral( "XX" )));
    scoreLabel->setText(i18n("Score: %1", QStringLiteral( "XXXXXX" )));
    filledLabel->setText(i18n( "Filled: %1%", QStringLiteral( "XX" )));
    livesLabel->setText(i18n( "Lives: %1", QStringLiteral( "XX" )));
    timeLabel->setText(i18n( "Time: %1", QStringLiteral( "XXX" )));
    
    m_statusBar->insertPermanentWidget(0, levelLabel, 1);
    m_statusBar->insertPermanentWidget(1, scoreLabel, 1);
    m_statusBar->insertPermanentWidget(2, filledLabel, 1);
    m_statusBar->insertPermanentWidget(3, livesLabel, 1);
    m_statusBar->insertPermanentWidget(4, timeLabel, 1);
        
    m_gameWidget = new KBounceGameWidget( this );
    connect( m_gameWidget, SIGNAL(levelChanged(int)), this, SLOT(displayLevel(int)) );
    connect( m_gameWidget, &KBounceGameWidget::scoreChanged, this, &KBounceMainWindow::displayScore );
    connect( m_gameWidget, &KBounceGameWidget::livesChanged, this, &KBounceMainWindow::displayLives );
    connect( m_gameWidget, &KBounceGameWidget::filledChanged, this, &KBounceMainWindow::displayFilled );
    connect( m_gameWidget, &KBounceGameWidget::timeChanged, this, &KBounceMainWindow::displayTime );
    connect( m_gameWidget, &KBounceGameWidget::stateChanged, this, &KBounceMainWindow::gameStateChanged );
    //connect( m_gameWidget, SIGNAL(gameOver()), this, SLOT(gameOverNow()) );
    setCentralWidget( m_gameWidget );

    initXMLUI();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setupGUI();

    readSettings();
}

KBounceMainWindow::~KBounceMainWindow()
{
}

/**
 * create the action events create the gui.
 */
void KBounceMainWindow::initXMLUI()
{
    // Game
    m_newAction = KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    KStandardGameAction::end(this, SLOT(closeGame()), actionCollection());
    m_pauseAction = KStandardGameAction::pause(this, SLOT(pauseGame()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscore()), actionCollection());
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());
  
    // Difficulty
    Kg::difficulty()->addStandardLevelRange(
        KgDifficultyLevel::Easy, KgDifficultyLevel::Hard
    );
    KgDifficultyGUI::init(this);
    connect(Kg::difficulty(), SIGNAL(currentLevelChanged(const KgDifficultyLevel*)), m_gameWidget, SLOT(levelChanged()));

    // Settings
    KStandardAction::preferences( this, SLOT(configureSettings()), actionCollection() );
    m_soundAction = new KToggleAction( i18n("&Play Sounds"), this );
    actionCollection()->addAction( QStringLiteral(  "toggle_sound" ), m_soundAction );
    connect( m_soundAction, &QAction::triggered, this, &KBounceMainWindow::setSounds );
}

		   
		   
void KBounceMainWindow::newGame()
{
	// Check for running game
    closeGame();
	if ( m_gameWidget->state() == KBounceGameWidget::BeforeFirstGame || m_gameWidget->state() == KBounceGameWidget::GameOver )
	{
		m_gameWidget->newGame();
	}
}

void KBounceMainWindow::pauseGame()
{
	if ( m_gameWidget->state() == KBounceGameWidget::Paused )
	{
		m_gameWidget->setPaused( false );
	}
    else
	{
		m_gameWidget->setPaused( true );
	}
}

void KBounceMainWindow::closeGame()
{
	if ( m_gameWidget->state() == KBounceGameWidget::BeforeFirstGame || m_gameWidget->state() == KBounceGameWidget::GameOver )
	{
		return;
	}

	KBounceGameWidget::State old_state = m_gameWidget->state();
	if ( old_state == KBounceGameWidget::Running )
		m_gameWidget->setPaused( true );
	int ret = KMessageBox::questionYesNo( this, i18n( "Do you really want to close the running game?" ), QString(),  KStandardGuiItem::yes(), KStandardGuiItem::cancel() );
	if ( ret == KMessageBox::Yes )
	{
		m_gameWidget->closeGame();
	}
	else if ( old_state == KBounceGameWidget::Running )
	{
		m_gameWidget->setPaused( false );
	}
}

void KBounceMainWindow::gameOverNow()
{
	statusBar()->showMessage(  i18n("Game over. Click to start a game") );
	highscore();
}

/**
 * Bring up the standard kde high score dialog.
 */
void KBounceMainWindow::showHighscore()
{
	KScoreDialog ksdialog( KScoreDialog::Name | KScoreDialog::Score, this );
    ksdialog.initFromDifficulty(Kg::difficulty());
	ksdialog.exec();
}

void KBounceMainWindow::highscore()
{
    if ( m_gameWidget->score() == 0 ) {
        return;
    }

    qCDebug(KBOUNCE_LOG);
    KScoreDialog ksdialog( KScoreDialog::Name | KScoreDialog::Score | KScoreDialog::Level, this );
    ksdialog.initFromDifficulty(Kg::difficulty());
    KScoreDialog::FieldInfo info;
    info[KScoreDialog::Score].setNum( m_gameWidget->score() );
    info[KScoreDialog::Level].setNum( m_gameWidget->level() );
    if ( ksdialog.addScore( info ) )
	ksdialog.exec();
}

void KBounceMainWindow::configureSettings()
{
    if ( KConfigDialog::showDialog( QStringLiteral("settings") ) ) return;

    KConfigDialog* dialog = new KConfigDialog( this, QStringLiteral("settings"), KBounceSettings::self());
    dialog->addPage( new KgThemeSelector(m_gameWidget->renderer()->themeProvider(), 0, dialog), i18n( "Theme" ), QStringLiteral("games-config-theme") );
    dialog->addPage( new BackgroundSelector(dialog,KBounceSettings::self() ),i18n("Background"),QStringLiteral("games-config-background"));
    dialog->show();
    connect( dialog, &KConfigDialog::settingsChanged, this, &KBounceMainWindow::settingsChanged );
}

void KBounceMainWindow::readSettings()
{
    m_soundAction->setChecked( KBounceSettings::playSounds() );
	m_gameWidget->settingsChanged();
}

void KBounceMainWindow::settingsChanged()
{
    m_gameWidget->settingsChanged();
    KBounceSettings::self()->save(); // Bug 184606
}

void KBounceMainWindow::setSounds( bool val )
{
    KBounceSettings::setPlaySounds( val );
    settingsChanged();
}

void KBounceMainWindow::displayLevel( int level )
{
    levelLabel->setText(i18n("Level: %1", level));
}

void KBounceMainWindow::displayScore( int score )
{
    scoreLabel->setText(i18n("Score: %1", score));
}

void KBounceMainWindow::displayFilled( int filled )
{
    filledLabel->setText(i18n("Filled: %1%", filled));
}

void KBounceMainWindow::displayLives( int lives )
{
    livesLabel->setText(i18n("Lives: %1", lives - 1));
}

void KBounceMainWindow::displayTime( int time )
{
    timeLabel->setText(i18n("Time: %1", time));
}

void KBounceMainWindow::gameStateChanged( KBounceGameWidget::State state )
{
    switch ( state )
    {
		case KBounceGameWidget::BeforeFirstGame :
			break;
		case KBounceGameWidget::BetweenLevels :
			break;
		case KBounceGameWidget::Suspended :
			break;
		case KBounceGameWidget::Paused :
		    m_pauseAction->setChecked( true );
		    m_statusBar->clearMessage();
		    break;
		case KBounceGameWidget::Running :
		    m_pauseAction->setChecked( false );
		    m_statusBar->clearMessage();
		    break;
		case KBounceGameWidget::GameOver :
		    statusBar()->showMessage(  i18n("Game over. Click to start a game") );
		    highscore();
		    break;
	    }
}

void KBounceMainWindow::focusOutEvent( QFocusEvent *ev )
{
    if ( m_gameWidget->state() == KBounceGameWidget::Running &&
         focusWidget() != m_gameWidget )
    {
        m_gameWidget->setPaused( true );
    }

    KXmlGuiWindow::focusOutEvent( ev );
}

void KBounceMainWindow::focusInEvent ( QFocusEvent *ev )
{
    //m_board->setSuspended( true );
    KXmlGuiWindow::focusInEvent( ev );
}

// void KBounceMainWindow::switchLevel()
// {
    /*
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

QString foo =
i18n("You have successfully cleared more than 75% of the board.\n") +
i18n("%1 points: 15 points per remaining life\n", m_level.lifes*15) +
i18n("%1 points: Bonus\n", (m_gameWidget->percent()-75)*2*(m_game.level+5)) +
i18n("%1 points: Total score for this level\n", score) +
i18n("On to level %1. Remember you get %2 lives this time!", m_game.level+1, m_game.level+2);

   KMessageBox::information( this,foo );


   // KMessageBox::information( this, i18n("You've completed level %1 with "
   //     "a score of %2.\nGet ready for the next one!").arg(level).arg(score));

    m_game.level++;
    m_levelLCD->display( m_game.level );

    createLevel( m_game.level );
    startLevel();
    */
// }



