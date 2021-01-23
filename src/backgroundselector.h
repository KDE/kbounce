/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2010 Andreas Scherf <ascherfy@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
