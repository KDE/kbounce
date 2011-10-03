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

#include <KLocale>
#include <KGameTheme>
#include <KColorScheme>

static const int GAME_TIME_DELAY = 1000;
static const int MIN_FILL_PERCENT = 75;
static const int POINTS_FOR_LIFE = 15;
static const int TICKS_PER_SECOND = 1000 / GAME_TIME_DELAY;

KBounceGameWidget::KBounceGameWidget( QWidget* parent )
    : KGameCanvasWidget( parent ), m_state( BeforeFirstGame ),
    m_bonus( 0 ), m_level( 0 ), m_lives( 0 ), m_time( 0 ), m_vertical( false )
{
    m_theme = new KGameTheme( "KGameTheme" );
    m_board = new KBounceBoard( &m_renderer, this, this );
    connect( m_board, SIGNAL(fillChanged(int)), this, SLOT(onFillChanged(int)) );
    connect( m_board, SIGNAL(wallDied()), this, SLOT(onWallDied()) );

    m_overlay = new KGameCanvasPixmap( this );
    m_overlay->raise();
    m_overlay->hide();

    m_clock = new QTimer( this );
    m_clock->setInterval( GAME_TIME_DELAY );
    connect( m_clock, SIGNAL(timeout()), this, SLOT(tick()) );
	connect( this, SIGNAL(livesChanged(int)),this,SLOT(onLivesChanged(int)) );

    setMouseTracking( true );
}

KBounceGameWidget::~KBounceGameWidget()
{
    delete m_board;
    delete m_overlay;
    delete m_theme;
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
        closeLevel();
        unsetCursor();
        m_state = GameOver;
        emit stateChanged( m_state );
        emit gameOver();

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
    
    m_board->setSounds( KBounceSettings::playSounds() );
	m_renderer.setTheme( KBounceSettings::theme() );

    if (KBounceSettings::useRandomBackgroundPictures())
    {
        m_renderer.setCustomBackgroundPath(KBounceSettings::backgroundPicturePath());
    }
    else
    {
        m_renderer.setCustomBackgroundPath(QString());
    }
   
    renderBackground();
    redraw();
}

void KBounceGameWidget::levelChanged(KGameDifficulty::standardLevel level)
{
    switch(level) {
        case KGameDifficulty::Easy:
            m_board->setBallVelocity(0.100);
            m_board->setWallVelocity(0.250);
            break;
        case KGameDifficulty::Medium:
            m_board->setWallVelocity(0.125);
            m_board->setBallVelocity(0.125);
            break;
        case KGameDifficulty::Hard:
            m_board->setWallVelocity(0.100);
            m_board->setBallVelocity(0.250);
            break;
        default:
            break;
    }
    m_difficultyLevel = level;
    KBounceSettings::setLevel(level);
    KBounceSettings::self()->writeConfig();
}

void KBounceGameWidget::setSounds( bool val )
{
    m_board->setSounds( val );
}

void KBounceGameWidget::setSoundPath( const QString& path )
{
    kDebug() << "sound path:" << path;
    m_board->setSoundPath( path );
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
	if ( lives < ( m_level + 1 ) )
	{
		m_board->playSound("timeout.wav");
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
    renderBackground();
    QSize boardSize( ev->size().width() - 30, ev->size().height() - 30 );
    m_board->resize( boardSize );
    m_board->moveTo( ( ev->size().width() - boardSize.width() ) / 2, ( ev->size().height() - boardSize.height() ) / 2 );

    redraw();
}

void KBounceGameWidget::renderBackground()
{
    QPalette palette;
    palette.setBrush( backgroundRole(), m_renderer.renderBackground() );
    setPalette( palette );
    setAutoFillBackground( true );
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
            QPoint pos( event->pos().x() - m_board->pos().x(), event->pos().y() - m_board->pos().y() );
            m_board->buildWall( pos, m_vertical );
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
    {
        m_renderer.loadNewBackgroundPixmap();
        renderBackground();
    }
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
    m_board->redraw();
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
	    text = i18n( "Paused" );
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
    m_overlay->moveTo( ( size().width() - itemWidth ) / 2, ( size().height() - itemHeight ) / 2 );
}


#include "gamewidget.moc"

