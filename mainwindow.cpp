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

#include <KAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <kdebug.h>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <krandom.h>
#include <KStatusBar>
#include <KStandardDirs>
#include <KToggleAction>

#include <kstandardgameaction.h>
#include <KScoreDialog>
#include <KgThemeSelector>
#include <KgDifficulty>


#include "gamewidget.h"
#include "settings.h"
#include "backgroundselector.h"

KBounceMainWindow::KBounceMainWindow()
{
    m_statusBar = statusBar();
    m_statusBar->insertItem( i18n( "Level: %1", QString::fromLatin1( "XX" ) ), 1, 1 );
    m_statusBar->insertItem( i18n( "Score: %1", QString::fromLatin1( "XXXXXX" ) ), 2, 1 );
    m_statusBar->insertItem( i18n( "Filled: %1%", QString::fromLatin1( "XX" ) ), 3, 1 );
    m_statusBar->insertItem( i18n( "Lives: %1", QString::fromLatin1( "XX" ) ), 4, 1 );
    m_statusBar->insertItem( i18n( "Time: %1", QString::fromLatin1( "XXX" ) ), 5, 1 );
        
    dirs = new KStandardDirs();
    userDataDir = dirs->saveLocation ("data", "kbounce", true);
    m_gameWidget = new KBounceGameWidget( this );
    
    connect( m_gameWidget, SIGNAL(levelChanged(int)), this, SLOT(displayLevel(int)) );
    connect( m_gameWidget, SIGNAL(scoreChanged(int)), this, SLOT(displayScore(int)) );
    connect( m_gameWidget, SIGNAL(livesChanged(int)), this, SLOT(displayLives(int)) );
    connect( m_gameWidget, SIGNAL(filledChanged(int)), this, SLOT(displayFilled(int)) );
    connect( m_gameWidget, SIGNAL(timeChanged(int)), this, SLOT(displayTime(int)) );
    connect( m_gameWidget, SIGNAL(stateChanged(KBounceGameWidget::State)), this, SLOT(gameStateChanged(KBounceGameWidget::State)) );
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
    m_saveGame = KStandardGameAction::save(this, SLOT(saveGame()), actionCollection());
    m_saveGame->setText (i18n ("&Save and Exit"));
    m_loadGame = KStandardGameAction::load(this, SLOT(loadGame()), actionCollection());
    m_loadGame->setText (i18n ("&Load last game"));
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
    actionCollection()->addAction( QLatin1String(  "toggle_sound" ), m_soundAction );
    connect( m_soundAction, SIGNAL(triggered(bool)), this, SLOT(setSounds(bool)) );
    
    //keys
    m_gameWidget->bindKeys(actionCollection());
}

bool KBounceMainWindow::safeRename (QWidget * theView, const QString & oldName,
                            const QString & newName)
{
    QFile newFile (newName);
    if (newFile.exists()) {
        // On some file systems we cannot rename if a file with the new name
        // already exists.  We must delete the existing file, otherwise the
        // upcoming QFile::rename will fail, according to Qt4 docs.  This
        // seems to be true with reiserfs at least.
        if (! newFile.remove()) {
            KMessageBox::information (theView, 
                i18n ("Cannot delete previous version of file '%1'.", 
		      i18n ("Rename File"), newName));
            return false;
        }
    }
    QFile oldFile (oldName);
    if (! oldFile.rename (newName)) {
        KMessageBox::information (theView, 
            i18n ("Cannot rename file '%1' to '%2'.", oldName, 
		  i18n ("Rename File"), newName));
        return false;
    }
    return true;
}
		   
		   
void KBounceMainWindow::newGame()
{
	// Check for running game
    closeGame();
	if ( m_gameWidget->state() == KBounceGameWidget::BeforeFirstGame || m_gameWidget->state() == KBounceGameWidget::GameOver )
	{
		m_gameWidget->newGame( 1, 0 );
	}
}

void KBounceMainWindow::saveGame()
{
    m_gameWidget->setPaused( true );

    QString saved;
    saved = saved.sprintf
                ("%1d %4d \n",
                 m_gameWidget->level(), m_gameWidget->score());

    QFile file1 (userDataDir + "savegame.dat");

    bool success = file1.open(QIODevice::WriteOnly);
    if (success) {
	QTextStream text1 (&file1);
	text1 << saved;
	KMessageBox::information (m_gameWidget,
            i18n ("Please note: For reasons of simplicity, your saved game "
            "position and score will be as they were at the start of this "
            "level, not as they are now. \nImportant: You are allowed "
	    "to load this saved game only once!"),
	    i18n ("Save Game"));
    }
    else {
	KMessageBox::information (m_gameWidget,
                                i18n ("Error: Failed to save your game."),
				 i18n ("Save Game"));
    }
    file1.close();
    m_gameWidget->closeGame();
}


void KBounceMainWindow::loadGame()
{
    QFile savedGames (userDataDir + "savegame.dat");
    if ( savedGames.exists() ){
	int ret = KMessageBox::questionYesNo( this, i18n( "Warning: All unsaved changes will be lost. Do you really want to close the running game?" ), QString(),  KStandardGuiItem::yes(), KStandardGuiItem::cancel() );

	if ( ret == KMessageBox::Yes ){
	    savedGames.open(QIODevice::ReadOnly);
	    QTextStream text1 ( &savedGames );
	    QString info = text1.readLine();

	    int lev = info.mid(0, 1).toInt();
	    int scr = info.mid(3, 4).toInt();

	    savedGames.close();
	    bool success = savedGames.remove( userDataDir + "savegame.dat");

	    if (success) {
	        m_gameWidget->newGame( lev, scr );
	    }
	}

	else {
	    m_gameWidget->setPaused( false );
	}
    }
    else {
        KMessageBox::information (m_gameWidget,
                         i18n ("Sorry, there are no saved games."),
			 i18n ("Load Game"));
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
	      int ret2 = KMessageBox::questionYesNo( this, i18n( "Do you want to save the game before closing?" ), QString(),  KStandardGuiItem::yes(), KStandardGuiItem::no() );

	      if ( ret2 == KMessageBox::Yes ) {
	      	  saveGame();
              }
	      else {
		  m_gameWidget->closeGame();
              }
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

    kDebug() ;
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
    if ( KConfigDialog::showDialog( "settings" ) ) return;

    KConfigDialog* dialog = new KConfigDialog( this, "settings", KBounceSettings::self());
    dialog->addPage( new KgThemeSelector(m_gameWidget->renderer()->themeProvider(), 0, dialog), i18n( "Theme" ), "games-config-theme" );
    dialog->addPage( new BackgroundSelector(dialog,KBounceSettings::self() ),i18n("Background"),"games-config-background");
    dialog->setHelp(QString(),"kbounce");
    dialog->show();
    connect( dialog, SIGNAL(settingsChanged(QString)), this, SLOT(settingsChanged()) );
}

void KBounceMainWindow::readSettings()
{
    m_soundAction->setChecked( KBounceSettings::playSounds() );
	m_gameWidget->settingsChanged();
}

void KBounceMainWindow::settingsChanged()
{
    m_gameWidget->settingsChanged();
    KBounceSettings::self()->writeConfig(); // Bug 184606
}

void KBounceMainWindow::setSounds( bool val )
{
    KBounceSettings::setPlaySounds( val );
    settingsChanged();
}

void KBounceMainWindow::displayLevel( int level )
{
    m_statusBar->changeItem( i18n( "Level: %1", level ), 1 );
}

void KBounceMainWindow::displayScore( int score )
{
    m_statusBar->changeItem( i18n( "Score: %1", score ), 2 );
}

void KBounceMainWindow::displayFilled( int filled )
{
    m_statusBar->changeItem( i18n( "Filled: %1%", filled ), 3 );
}

void KBounceMainWindow::displayLives( int lives )
{
    m_statusBar->changeItem( i18n( "Lives: %1", lives - 1 ), 4 );
}

void KBounceMainWindow::displayTime( int time )
{
    m_statusBar->changeItem( i18n( "Time: %1", time ), 5 );
}

void KBounceMainWindow::gameStateChanged( KBounceGameWidget::State state )
{
    switch ( state ) {
        case KBounceGameWidget::BeforeFirstGame :
        case KBounceGameWidget::BetweenLevels :
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


#include "mainwindow.moc"
