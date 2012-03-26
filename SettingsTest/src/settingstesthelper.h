#ifndef SETTINGSTESTHELPER_H
#define SETTINGSTESTHELPER_H

#include <QObject>

class SettingsTestHelper : public QObject
{
    Q_OBJECT

public:
    SettingsTestHelper(QObject *parent = 0);
    ~SettingsTestHelper();

public slots:
      void runTest();    
};

#endif // SETTINGSTESTHELPER_H
