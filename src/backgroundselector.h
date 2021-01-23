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

#ifndef BACKGROUNDSELECTOR_H
#define BACKGROUNDSELECTOR_H

#include <QWidget>
#include <KConfigSkeleton>
namespace Ui {
    class KBounceBackgroundSelector;
}

class BackgroundSelector : public QWidget {
    Q_OBJECT
    public:
        explicit BackgroundSelector(QWidget *parent ,KConfigSkeleton * config );
        ~BackgroundSelector() override;

        void setupData();
    private Q_SLOTS:
        void imagePathChanged(const QString& path);
        void useRandomBackgroundPicturesChanged(bool state);
        void previewBackgroundPicture();
    protected:
        void changeEvent(QEvent *e) override;
        void enableSettings(bool enable=true);
    private:
        Ui::KBounceBackgroundSelector *ui;
        KConfigSkeleton * m_config;
};

#endif // BACKGROUNDSELECTOR_H
