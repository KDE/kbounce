/*
* Copyright (C) 2010 Andreas Scherf <ascherfy@gmail.com>
*
* This file is part of the KDE project "KBounce"
*
* KBounce is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* KBounce is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with KBounce; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
*/

#include "backgroundselector.h"
#include "ui_backgroundselector.h"

#include <KConfigDialog>
#include <KColorScheme>
#include <KLocalizedString>

#include "renderer.h"

#include <QPainter>
#include <QFileDialog>

BackgroundSelector::BackgroundSelector(QWidget* parent, KConfigSkeleton* config) :
    QWidget(parent),
    ui(new Ui::KBounceBackgroundSelector),m_config(config)
{
    ui->setupUi(this);
    ui->kurlrequester->fileDialog()->setFileMode(QFileDialog::Directory);
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
        connect(ui->kurlrequester, SIGNAL(textChanged(QString)), this, SLOT(imagePathChanged(QString)));
        connect(ui->kcfg_UseRandomBackgroundPictures, SIGNAL(toggled(bool)),this,SLOT(useRandomBackgroundPicturesChanged(bool)));

        //Get our currently configured Tileset entry
        KConfig * config = m_config->config();
        KConfigGroup group = config->group("General");
        QString picturePath = group.readPathEntry("BackgroundPicturePath",QDir::homePath() );
        ui->kurlrequester->setUrl(picturePath);
        ui->kurlrequester->setStartDir(picturePath);
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




