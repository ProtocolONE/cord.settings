#ifndef SETTINGSSAVER_H
#define SETTINGSSAVER_H

#include <QObject>
#include <qtimer.h>

class SettingsSaver : public QObject
{
    Q_OBJECT

public:
    SettingsSaver(QObject *parent = 0);
    ~SettingsSaver();

private slots:
    void sync();

private:
    QTimer timer;
};

#endif // SETTINGSSAVER_H
