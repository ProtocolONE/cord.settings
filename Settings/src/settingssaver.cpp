#include "settingssaver.h"
#include "settings.h"
#include <qDebug>

namespace GGS {
    namespace Settings {

        SettingsSaver::SettingsSaver(QObject *parent)
            : QObject(parent)
        {
            timer.setInterval(1000);
            connect(&timer, SIGNAL(timeout()), this, SLOT(sync()));
            timer.start();
        }

        SettingsSaver::~SettingsSaver()
        {

        }

        void SettingsSaver::sync()
        {
            Settings::sync();
        }

    }
}