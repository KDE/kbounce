/*
    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gamewidget.h"
#include "settings.h"
#include "wall.h"
#include "debug.h"

#include <QPalette>
#include <QTimer>
#include <QStandardPaths>

#include <KgThemeProvider>
#include <KColorScheme>
#include <KLocalizedString>

static const int MIN_MARGIN = 50;
static const int GAME_TIME_DELAY = 1000;
static const int MIN_FILL_PERCENT = 75;
static const int POINTS_FOR_LIFE = 15;
static const int TICKS_PER_SECOND = 1000 / GAME_TIME_DELAY;

KBounceGameWidget::KBounceGameWidget( QWidget* parent )
    : QGraphicsView( parent )
    , m_state( BeforeFirstGame )
    , m_bonus( 0 )
    , m_level( 0 )
    , m_lives( 0 )
    , m_time( 0 )
    , m_vertical( false )
    , m_soundTimeout( QStandardPaths::locate( QStandardPaths::AppDataLocation, QStringLiteral("sounds/timeout.wav") ) )
{
    m_board = new KBounceBoard( &m_renderer );
    connect(m_board, &KBounceBoard::fillChanged, this, &KBounceGameWidget::onFillChanged);
    connect(m_board, &KBounceBoard::wallDied, this, &KBounceGameWidget::onWallDied);

    m_overlay = new QGraphicsPixmapItem();
    m_overlay->hide();

    m_clock = new QTimer( this );
    m_clock->setInterval( GAME_TIME_DELAY );
    connect(m_clock, &QTimer::timeout, this, &KBounceGameWidget::tick);
    connect(this, &KBounceGameWidget::livesChanged, this, &KBounceGameWidget::onLivesChanged);

    setMouseTracking( true );

    connect(m_renderer.themeProvider(), &KgThemeProvider::currentThemeChanged,
            this, &KBounceGameWidget::settingsChanged);

    m_scene.addItem( m_board );
    m_scene.addItem( m_overlay );
    setScene( &m_scene );
}

KBounceGameWidget::~KBounceGameWidget()
{
    delete m_board;
    delete m_overlay;
}

int KBounceGameWidget::level()
{
    return m_level;
}

int KBounceGameWidget::score()
{
    qCDebug(KBOUNCE_LOG) << "Score:" << m_score;
    return m_score;
}

QSize KBounceGameWidget::minimumSizeHint() const
{
    return QSize( 576, 384 );
}

void KBounceGameWidget::closeGame()
{
    if ( m_state != BeforeFirstGame && m_state != GameOver )
    {
        m_clock->stop();
        m_board->setPaused( true );
        m_state = GameOver;
        Q_EMIT stateChanged( m_state );
        Q_EMIT gameOver();

        Kg::difficulty()->setGameRunning( false );
        redraw();
    }
}

void KBounceGameWidget::newGame()
{
    closeGame();
    m_level = 1;
    m_score = 0;

    Q_EMIT levelChanged( m_level );
    Q_EMIT scoreChanged( m_score );

    Kg::difficulty()->setGameRunning( true );
    newLevel();
}

void KBounceGameWidget::setPaused( bool val )
{
    if ( m_state == Paused && val == false )
    {
        m_clock->start();
        m_board->setPaused( false );
        m_state = Running;
        Q_EMIT stateChanged( m_state );
    }
    else if ( m_state == Running && val == true )
    {
        m_clock->stop();
        m_board->setPaused( true );
        m_state = Paused;
        Q_EMIT stateChanged( m_state );
    }

    redraw();
}

void KBounceGameWidget::setSuspended( bool val )
{
    if ( m_state == Suspended && val == false )
    {
        m_clock->start();
        m_board->setPaused( false );
        m_state = Running;
        Q_EMIT stateChanged( m_state );
    }

    if ( m_state == Running && val == true )
    {
        m_clock->stop();
        m_board->setPaused( true );
        m_state = Suspended;
        Q_EMIT stateChanged( m_state );
    }
    redraw();
}

void KBounceGameWidget::settingsChanged()
{
    qCDebug(KBOUNCE_LOG) << "Settings changed";

    if (KBounceSettings::useRandomBackgroundPictures())
    {
        m_renderer.setCustomBackgroundPath(KBounceSettings::backgroundPicturePath());
    }
    else
    {
        m_renderer.setCustomBackgroundPath(QString());
    }
    redraw();
}

void KBounceGameWidget::setGameDifficulty( const KgDifficultyLevel * difficulty )
{
    switch ( difficulty->hardness() ) {
        case KgDifficultyLevel::Easy:
            m_board->setBallVelocity(0.100);
            m_board->setWallVelocity(0.250);
            break;
        case KgDifficultyLevel::Medium:
            m_board->setWallVelocity(0.125);
            m_board->setBallVelocity(0.125);
            break;
        case KgDifficultyLevel::Hard:
            m_board->setWallVelocity(0.100);
            m_board->setBallVelocity(0.250);
            break;
    }
}

void KBounceGameWidget::handleLevelChanged()
{
    setGameDifficulty( Kg::difficulty()->currentLevel() );

    if ( m_state == Running || m_state == Paused )
        newGame();
}

void KBounceGameWidget::onFillChanged( int fill )
{
    Q_EMIT filledChanged( fill );
    if ( fill >= MIN_FILL_PERCENT )
    {
        closeLevel();
        m_level++;
        Q_EMIT levelChanged( m_level );

        m_state = BetweenLevels;
        Q_EMIT stateChanged( m_state );

        redraw();
    }
}

void KBounceGameWidget::onWallDied()
{
    if ( m_lives <= 1 )
    {
        closeGame();
    }
    else
    {
        m_lives--;
        Q_EMIT livesChanged( m_lives );
    }
}

void KBounceGameWidget::onLivesChanged(int lives)
{
    if ( lives < ( m_level + 1 )
            && KBounceSettings::playSounds() )
    {
        m_soundTimeout.start();
    }
}


void KBounceGameWidget::tick()
{
    static int ticks = TICKS_PER_SECOND;
    ticks--;
    if ( ticks <= 0 )
    {
        Q_EMIT timeChanged( --m_time );
        if ( m_time == 0 )
        {
            closeGame();
        }
        ticks = TICKS_PER_SECOND;
    }
}

void KBounceGameWidget::resizeEvent( QResizeEvent* ev )
{
    qCDebug(KBOUNCE_LOG) << "Size" << ev->size();

    m_renderer.setBackgroundSize( ev->size() );

    QRectF rect( 0, 0, ev->size().width(), ev->size().height() );
    m_scene.setSceneRect( rect );

    QSize boardSize( sceneRect().width()  - MIN_MARGIN,
            sceneRect().height() - MIN_MARGIN );
    m_board->resize( boardSize );

    qreal x = ( sceneRect().width()  - m_board->boundingRect().width() )  / 2;
    qreal y = ( sceneRect().height() - m_board->boundingRect().height() ) / 2;
    m_board->setPos( x, y );

    redraw();
}

void KBounceGameWidget::mouseReleaseEvent( QMouseEvent* event )
{
    if ( event->button() & Qt::RightButton )
    {
        m_vertical = !m_vertical;
        updateCursor();
    }

    if ( event->button() & Qt::LeftButton )
    {
        if ( m_state == Running )
        {
            m_board->buildWall( mapToScene( event->pos() ), m_vertical );
        }
        else if ( m_state == Paused )
        {
            setPaused( false );
        }
        else if ( m_state == BetweenLevels )
        {
            newLevel();
        }
        else if ( m_state == BeforeFirstGame || m_state == GameOver )
        {
            newGame();
        }
    }
}


void KBounceGameWidget::closeLevel()
{
    m_bonus = 0;
    if ( m_board->filled() >= MIN_FILL_PERCENT )
    {
        m_bonus = ( m_board->filled() - MIN_FILL_PERCENT ) * 2 * ( m_level + 5 );
    }
    m_score += m_bonus;
    m_score += POINTS_FOR_LIFE * m_lives;
    Q_EMIT scoreChanged( m_score );

    m_clock->stop();
    m_board->setPaused( true );
}

void KBounceGameWidget::newLevel()
{
    m_state = Running;
    Q_EMIT stateChanged( m_state );

    m_clock->start();
    m_board->newLevel( m_level );
    m_board->setPaused( false );

    m_bonus = 0;
    m_lives = m_level + 1;
    m_time = 30 * ( m_level + 2 );
    Q_EMIT livesChanged( m_lives );
    Q_EMIT timeChanged( m_time );

    if (KBounceSettings::useRandomBackgroundPictures())
        m_renderer.loadNewBackgroundPixmap();

    redraw();
}


void KBounceGameWidget::redraw()
{
    if ( size().isEmpty() )
        return;

    switch ( m_state )
    {
        case BeforeFirstGame:
            m_board->hide();
            generateOverlay();
            m_overlay->show();
            break;
        case Running:
            m_board->show();
            m_overlay->hide();
            break;
        default:
            m_board->show();
            generateOverlay();
            m_overlay->show();
            break;
    }

    updateCursor();
    KBounceWall::loadSprites();
    m_scene.setBackgroundBrush( m_board->applyWallsOn(m_renderer.renderBackground()) );
    update();
}

void KBounceGameWidget::generateOverlay()
{
    if ( size().isEmpty() )
        return;

    int itemWidth = qRound( 0.8 * size().width() );
    int itemHeight = qRound( 0.6 * size().height() );

    QSize backgroundSize( itemWidth,itemHeight );

    QPixmap px( backgroundSize );
    px.fill( Qt::transparent );

    QPainter p( &px );

    p.setPen( Qt::transparent );
    p.setRenderHint(QPainter::Antialiasing );

    if ( m_renderer.spriteExists(QStringLiteral("overlayBackground")) )
    {
        QPixmap themeBackgound = m_renderer.spritePixmap(QStringLiteral("overlayBackground"),backgroundSize);
        p.setCompositionMode( QPainter::CompositionMode_Source );
        p.drawPixmap( p.viewport(), themeBackgound );
        p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
        p.fillRect(px.rect(), QColor( 0, 0, 0, 160 ));
        p.setCompositionMode( QPainter::CompositionMode_SourceOver );
    }
    else
    {
        p.setBrush( QBrush( QColor( 188, 202, 222, 155 ) ) );
        p.drawRoundedRect( 0, 0, itemWidth, itemHeight, 25, 25 );
    }

    QString text;
    switch( m_state )
    {
        case BeforeFirstGame:
            text = i18n( "Welcome to KBounce.\n Click to start a game" );
            break;
        case Paused:
            text = i18n( "Paused\n Click to resume" );
            break;
        case BetweenLevels:
            text = i18n( "You have successfully cleared more than %1% of the board\n", MIN_FILL_PERCENT ) +
                i18n( "%1 points: %2 points per remaining life\n", m_lives * POINTS_FOR_LIFE, POINTS_FOR_LIFE ) +
                i18n( "%1 points: Bonus\n", m_bonus ) +
                i18n( "%1 points: Total score for this level\n", m_bonus + m_lives * POINTS_FOR_LIFE ) +
                i18n( "On to level %1. Remember you get %2 lives this time!", m_level, m_level + 1 );
            break;
        case GameOver:
            text = i18n( "Game over.\n Click to start a game" );
            break;
        default:
            text = QString();
    }

    QFont font;
    font.setPointSize( 28 );
    p.setFont( font );
    int textWidth = p.boundingRect( p.viewport(), Qt::AlignCenter | Qt::AlignVCenter, text ).width();
    int fontSize = 28;
    while ( ( textWidth > itemWidth * 0.95 ) && fontSize > 1 )
    {
        fontSize--;
        font.setPointSize( fontSize );
        p.setFont( font );
        textWidth = p.boundingRect( p.viewport(), Qt::AlignCenter | Qt::AlignVCenter, text ).width();
    }
    KColorScheme kcs = KColorScheme( QPalette::Normal, KColorScheme::Window );
    p.setPen( kcs.foreground(KColorScheme::NormalText).color());
    p.drawText( p.viewport(), Qt::AlignCenter | Qt::AlignVCenter, text );
    p.end();

    m_overlay->setPixmap( px );

    QPointF pos( ( sceneRect().width() - itemWidth) / 2,
            ( sceneRect().height() - itemHeight) / 2 );
    m_overlay->setPos( pos );
}

void KBounceGameWidget::focusOutEvent(QFocusEvent *event)
{
    if (event->reason() == Qt::ActiveWindowFocusReason)
    {
        setPaused( true );
    }
}

void KBounceGameWidget::updateCursor()
{
    if ( m_state == Running )
        setCursor( m_vertical ? Qt::SizeVerCursor : Qt::SizeHorCursor );
    else
        unsetCursor();
}



