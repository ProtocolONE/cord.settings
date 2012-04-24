#ifndef SETTINGSSAVER_H
#define SETTINGSSAVER_H

#include <QObject>
#include <qtimer.h>
#include "Settings_p.h"
namespace GGS {
    namespace Settings {

        class SETTINGSLIB_EXPORT SettingsSaver : public QObject
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
    }
}
#endif // SETTINGSSAVER_H
