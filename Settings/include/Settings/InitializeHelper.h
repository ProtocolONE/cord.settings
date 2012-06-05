/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_SETTINGS_INITIALIZEHELPER_H
#define _GGS_SETTINGS_INITIALIZEHELPER_H

#include <Settings/settings_global.h>

#include <QtCore/QString>

class QSqlDatabase;

namespace GGS {
  namespace Settings {

    /*!
      \class InitializeHelper
    
      \brief Initialize helper. 
      \code
        InitializeHelper helper;
        helper.setUserName("admin");
        helper.setPassword("pass");
        helper.setPath("c:\\db.sql");
        if (!helper.init()) {
          //TODO Fatal error
        }
        

        if (helper.isRecreated()) {
          //TODO Make first time initialization
        }
      \endcode
    */

    class SETTINGSLIB_EXPORT InitializeHelper
    {
    public:
      InitializeHelper();
      virtual ~InitializeHelper();

      const QString &userName() const;
      void setUserName(QString &val);

      const QString &password() const;
      void setPassword(QString &val);

      const QString &fileName() const;
      void setFileName(QString &val);

      const QString &connectionName() const;
      void setConnectionName(QString &val);

      bool isRecreated();

      bool init();
    private:
      inline bool recreateDb(QSqlDatabase *db);
      inline bool createSettingsTable(QSqlDatabase *db);
      inline bool isSettingsDatabaseDamaged();

      QString _userName;
      QString _password;
      QString _fileName;
      QString _connectionName;
      bool _recreate;
    };
  }
}

#endif // _GGS_SETTINGS_INITIALIZEHELPER_H