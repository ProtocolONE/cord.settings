#include <Settings/SettingsSaver.h>
#include <Settings/Settings.h>

namespace GGS {
  namespace Settings {

    SettingsSaver::SettingsSaver(QObject *parent)
      : QObject(parent)
    {
      this->timer.setInterval(1000);
      connect(&this->timer, SIGNAL(timeout()), this, SLOT(sync()));
      this->timer.start();
    }

    SettingsSaver::~SettingsSaver()
    {
      this->timer.stop();
    }
    
    
    void SettingsSaver::sync()
    {
      Settings::sync();
    }
  }
}