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

#include "gamewidget.h"
#include "settings.h"

#include <QPalette>
#include <QTimer>

#include <KStandardDirs>
#include <KLocale>
#include <KgDifficulty>
#include <KgThemeProvider>
#include <KColorScheme>

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
, m_soundTimeout( KStandardDirs::locate( "appdata", "sounds/timeout.wav" ) )
{
    m_board = new KBounceBoard( &m_renderer );
    connect( m_board, SIGNAL(fillChanged(int)), this, SLOT(onFillChanged(int)) );
    connect( m_board, SIGNAL(wallDied()), this, SLOT(onWallDied()) );

    m_overlay = new QGraphicsPixmapItem();
    m_overlay->hide();

    m_clock = new QTimer( this );
    m_clock->setInterval( GAME_TIME_DELAY );
    connect( m_clock, SIGNAL(timeout()), this, SLOT(tick()) );
	connect( this, SIGNAL(livesChanged(int)),this,SLOT(onLivesChanged(int)) );

    setMouseTracking( true );

    connect(m_renderer.themeProvider(), SIGNAL(currentThemeChanged(const KgTheme*)),
        SLOT(settingsChanged()));

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
    kDebug() << "Score:" << m_score;
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
        unsetCursor();
        m_state = GameOver;
        emit stateChanged( m_state );
        emit gameOver();

        Kg::difficulty()->setGameRunning( false );
        redraw();
    }
}

void KBounceGameWidget::newGame()
{
    closeGame();
    setCursor( m_vertical ? Qt::SizeVerCursor : Qt::SizeHorCursor );
    m_level = 1;
    m_score = 0;

    emit levelChanged( m_level );
    emit scoreChanged( m_score );

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
        emit stateChanged( m_state );
    }
    else if ( m_state == Running && val == true )
    {
        m_clock->stop();
        m_board->setPaused( true );
        m_state = Paused;
        emit stateChanged( m_state );
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
        emit stateChanged( m_state );
    }

    if ( m_state == Running && val == true )
    {
        m_clock->stop();
        m_board->setPaused( true );
        m_state = Suspended;
        emit stateChanged( m_state );
    }
    redraw();
}

void KBounceGameWidget::settingsChanged()
{
    kDebug() << "Settings changed";

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

void KBounceGameWidget::levelChanged()
{
    setGameDifficulty( Kg::difficulty()->currentLevel() );

    if (m_state == Running)
        newGame();
}

void KBounceGameWidget::onFillChanged( int fill )
{
    emit filledChanged( fill );
    if ( fill >= MIN_FILL_PERCENT )
    {
        closeLevel();
        m_level++;
        emit levelChanged( m_level );

        m_state = BetweenLevels;
        emit stateChanged( m_state );

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
        emit livesChanged( m_lives );
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
        if ( m_time == 1 )
            closeGame();
        else
        {
            m_time--;
            emit timeChanged( m_time );
        }
        ticks = TICKS_PER_SECOND;
    }
}

void KBounceGameWidget::resizeEvent( QResizeEvent* ev )
{
    kDebug() << "Size" << ev->size();

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
        setCursor( m_vertical ? Qt::SizeVerCursor : Qt::SizeHorCursor );
    }

    if ( event->button() & Qt::LeftButton )
    {
        if ( m_state == Running )
        {
            m_board->buildWall( mapToScene( event->pos() ), m_vertical );
        }
        else if ( m_state == Paused )
        {
            setPaused(false);
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
    emit scoreChanged( m_score );

    m_clock->stop();
	m_board->setPaused( true );
}

void KBounceGameWidget::newLevel()
{
    m_state = Running;
    emit stateChanged( m_state );

    m_clock->start();
    m_board->newLevel( m_level );
    m_board->setPaused( false );

    m_bonus = 0;
    m_lives = m_level + 1;
    m_time = 30 * ( m_level + 2 );
    emit livesChanged( m_lives );
    emit timeChanged( m_time );

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

    m_scene.setBackgroundBrush( m_renderer.renderBackground() );
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
	
	if ( m_renderer.spriteExists("overlayBackground") )
	{
		QPixmap themeBackgound = m_renderer.spritePixmap("overlayBackground",backgroundSize);
		p.setCompositionMode( QPainter::CompositionMode_Source );
		p.drawPixmap( p.viewport(), themeBackgound );
		p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
		p.fillRect(px.rect(), QColor( 0, 0, 0, 160 ));
		p.setCompositionMode( QPainter::CompositionMode_SourceOver );
	}
	else
	{
		p.setBrush( QBrush( QColor( 188, 202, 222, 155 ) ) );
		p.drawRoundRect( 0, 0, itemWidth, itemHeight, 25 );
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
        setPaused(true);
    }
}


#include "gamewidget.moc"

