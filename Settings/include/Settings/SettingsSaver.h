/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_SETTINGS_SETTINGSSAVER_H
#define _GGS_SETTINGS_SETTINGSSAVER_H

#include <Settings/settings_global.h>

#include <QtCore/QObject>
#include <QtCore/QTimer>

namespace GGS {
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
#endif // _GGS_SETTINGS_SETTINGSSAVER_H
