/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2010 Andreas Scherf <ascherfy@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "backgroundselector.h"
#include "ui_backgroundselector.h"

#include <KColorScheme>
#include <KLocalizedString>

#include "renderer.h"

#include <QPainter>

BackgroundSelector::BackgroundSelector(QWidget* parent, KConfigSkeleton* config) :
    QWidget(parent),
    ui(new Ui::KBounceBackgroundSelector),m_config(config)
{
    ui->setupUi(this);
    setupData();
}

BackgroundSelector::~BackgroundSelector()
{
    delete ui;
}

void BackgroundSelector::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}


void BackgroundSelector::setupData()
{
    if (m_config)
    {
        //The lineEdit widget holds our bg path, but the user does not manipulate it directly
        ui->kcfg_BackgroundPicturePath->hide();
        connect(ui->kurlrequester, &KUrlRequester::textChanged, this, &BackgroundSelector::imagePathChanged);
        connect(ui->kcfg_UseRandomBackgroundPictures, &QCheckBox::toggled, this, &BackgroundSelector::useRandomBackgroundPicturesChanged);

        //Get our currently configured Tileset entry
        KConfig * config = m_config->config();
        KConfigGroup group = config->group("General");
        QString picturePath = group.readPathEntry("BackgroundPicturePath",QDir::homePath() );
        ui->kurlrequester->setUrl(QUrl(picturePath));
        ui->kurlrequester->setStartDir(QUrl(picturePath));
    }
}

void BackgroundSelector::useRandomBackgroundPicturesChanged(bool toggled)
{
    if (toggled)
    {
        enableSettings(true);
        previewBackgroundPicture();
    }
    else
    {
        enableSettings(false);
    }
}

void BackgroundSelector::enableSettings(bool enable)
{
    ui->kurlrequester->setEnabled(enable);
    ui->backgroundPicturePreview->setEnabled(enable);
}

void BackgroundSelector::imagePathChanged(const QString& path)
{
    ui->kcfg_BackgroundPicturePath->setText(path);
    previewBackgroundPicture();
}

void BackgroundSelector::previewBackgroundPicture()
{
    KBounceRenderer render;
    QSize previewSize = ui->backgroundPicturePreview->size();
    QPixmap previewPixmap = render.getRandomBackgroundPixmap(ui->kcfg_BackgroundPicturePath->text());
    if (previewPixmap.isNull())
    {
        previewPixmap = QPixmap(previewSize);
        previewPixmap.fill( Qt::transparent );
        QPainter p( &previewPixmap );
        QString text=i18n("No background pictures found.");
        QFont font;
        font.setPointSize( 11 );
        p.setFont( font );
        KColorScheme kcs(QPalette::Normal,KColorScheme::Window);
        p.setPen(kcs.foreground(KColorScheme::NeutralText).color());
        p.drawText( p.viewport(), Qt::AlignCenter | Qt::AlignTop | Qt::TextWordWrap, text );
        p.end();
    }
    ui->backgroundPicturePreview->setPixmap(previewPixmap.scaled(previewSize,Qt::KeepAspectRatio,Qt::SmoothTransformation));
}




