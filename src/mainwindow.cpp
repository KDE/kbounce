/*
    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindow.h"
#include "settings.h"
#include "backgroundselector.h"
#include "debug.h"
// KDEGames
#include <KGameStandardAction>
#include <KGameHighScoreDialog>
#include <KGameThemeSelector>
#include <KGameDifficulty>
// KF
#include <KStandardGuiItem>
#include <KToggleAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KMessageBox>
#include <KLocalizedString>
// Qt
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
    connect( m_gameWidget, &KBounceGameWidget::levelChanged, this, &KBounceMainWindow::displayLevel );
    connect( m_gameWidget, &KBounceGameWidget::scoreChanged, this, &KBounceMainWindow::displayScore );
    connect( m_gameWidget, &KBounceGameWidget::livesChanged, this, &KBounceMainWindow::displayLives );
    connect( m_gameWidget, &KBounceGameWidget::filledChanged, this, &KBounceMainWindow::displayFilled );
    connect( m_gameWidget, &KBounceGameWidget::timeChanged, this, &KBounceMainWindow::displayTime );
    connect( m_gameWidget, &KBounceGameWidget::stateChanged, this, &KBounceMainWindow::gameStateChanged );
    //connect( m_gameWidget, SIGNAL(gameOver()), this, SLOT(gameOverNow()) );
    setCentralWidget( m_gameWidget );

    initXMLUI();

    /* Set initial ball and wall velocity according to the difficulty */
    m_gameWidget->handleLevelChanged();

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
    m_newAction = KGameStandardAction::gameNew(this, &KBounceMainWindow::newGame, actionCollection());
    KGameStandardAction::end(this, &KBounceMainWindow::closeGame, actionCollection());
    m_pauseAction = KGameStandardAction::pause(this, &KBounceMainWindow::pauseGame, actionCollection());
    KGameStandardAction::highscores(this, &KBounceMainWindow::showHighscore, actionCollection());
    KGameStandardAction::quit(this, &QWidget::close, actionCollection());

    // Difficulty
    KGameDifficulty::global()->addStandardLevelRange(
            KGameDifficultyLevel::Easy, KGameDifficultyLevel::Hard
            );
    KGameDifficultyGUI::init(this);
    connect(KGameDifficulty::global(), &KGameDifficulty::currentLevelChanged,
            m_gameWidget, &KBounceGameWidget::handleLevelChanged);

    // Settings
    KStandardAction::preferences( this, &KBounceMainWindow::configureSettings, actionCollection() );
    m_soundAction = new KToggleAction( i18nc("@option:check", "Play Sounds"), this );
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
    int ret = KMessageBox::questionTwoActions(this,
                                         i18n("Are you sure you want to quit the current game?"),
                                         QString(),
                                         KGuiItem(i18nc("@action;button", "Quit Game"), QStringLiteral("window-close")),
                                         KStandardGuiItem::cancel() );
    if ( ret == KMessageBox::PrimaryAction )
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
    KGameHighScoreDialog ksdialog( KGameHighScoreDialog::Name | KGameHighScoreDialog::Score, this );
    ksdialog.initFromDifficulty(KGameDifficulty::global());
    ksdialog.exec();
}

void KBounceMainWindow::highscore()
{
    if ( m_gameWidget->score() == 0 ) {
        return;
    }

    qCDebug(KBOUNCE_LOG);
    KGameHighScoreDialog ksdialog( KGameHighScoreDialog::Name | KGameHighScoreDialog::Score | KGameHighScoreDialog::Level, this );
    ksdialog.initFromDifficulty(KGameDifficulty::global());
    KGameHighScoreDialog::FieldInfo info;
    info[KGameHighScoreDialog::Score].setNum( m_gameWidget->score() );
    info[KGameHighScoreDialog::Level].setNum( m_gameWidget->level() );
    if ( ksdialog.addScore( info ) )
        ksdialog.exec();
}

void KBounceMainWindow::configureSettings()
{
    if ( KConfigDialog::showDialog( QStringLiteral("settings") ) ) return;

    KConfigDialog* dialog = new KConfigDialog( this, QStringLiteral("settings"), KBounceSettings::self());
    dialog->addPage( new KGameThemeSelector(m_gameWidget->renderer()->themeProvider(), KGameThemeSelector::DefaultBehavior, dialog), i18nc("@title:tab", "Theme"), QStringLiteral("games-config-theme") );
    dialog->addPage( new BackgroundSelector(dialog,KBounceSettings::self() ),i18nc("@title:tab", "Background"),QStringLiteral("games-config-background"));
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

#include "moc_mainwindow.cpp"
