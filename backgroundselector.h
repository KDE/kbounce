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
    ~BackgroundSelector();

     void setupData();
private slots:
    void imagePathChanged(const QString& path);
    void useRandomBackgroundPicturesChanged(bool state);
    void previewBackgroundPicture();
protected:
    void changeEvent(QEvent *e);
    void enableSettings(bool enable=true);
private:
    Ui::KBounceBackgroundSelector *ui;
    KConfigSkeleton * m_config;
};

#endif // BACKGROUNDSELECTOR_H
