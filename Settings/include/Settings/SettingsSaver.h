#pragma once

#include <Settings/settings_global.h>

#include <QtCore/QObject>
#include <QtCore/QTimer>

namespace P1 {
  namespace Settings {

    class SETTINGSLIB_EXPORT SettingsSaver : public QObject
    {
      Q_OBJECT
    public:
      explicit SettingsSaver(QObject *parent = 0);
      ~SettingsSaver();
      
    private slots:
      void sync();

    private:
      QTimer timer;
    };
  }
}
