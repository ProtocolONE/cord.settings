#include <Settings/InitializeHelper.h>
#include <Settings/Settings.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

namespace GGS {
  namespace Settings {
    InitializeHelper::InitializeHelper()
      : _recreate(false),
        _userName("admin"),
        _password("admin"),
        _fileName("settings.sql"),
        _connectionName("settings")
    {
    }

    InitializeHelper::~InitializeHelper()
    {
    }

    const QString &InitializeHelper::userName() const
    {
      return this->_userName;
    }

    void InitializeHelper::setUserName(QString &val)
    {
      this->_userName = val;
    }

    const QString &InitializeHelper::password() const
    {
      return this->_password;
    }

    void InitializeHelper::setPassword(QString &val)
    {
      this->_password = val;
    }

    const QString &InitializeHelper::fileName() const
    {
      return this->_fileName;
    }

    void InitializeHelper::setFileName(QString &val)
    {
      this->_fileName = val;
    }

    const QString &InitializeHelper::connectionName() const
    {
      return this->_connectionName;
    }

    void InitializeHelper::setConnectionName(QString &val)
    {
      this->_connectionName = val;
    }
    
    bool InitializeHelper::isRecreated()
    {
      return this->_recreate;
    }

    bool InitializeHelper::init()
    {
      Q_ASSERT(this->_fileName.length());

      this->_recreate = false;

      QSqlDatabase db;
      if (QSqlDatabase::contains(this->_connectionName)) {
         db = QSqlDatabase::database(this->_connectionName);
      } else {
        db = QSqlDatabase::addDatabase("QSQLITE", this->_connectionName);
        db.setDatabaseName(this->_fileName);
      }

      bool recreateDb = false;
      if (db.open(this->_userName, this->_password)) {
        if (db.tables().contains("app_settings")) {
          Settings::setConnection(db.connectionName());
          if (this->isSettingsDatabaseDamaged())
            recreateDb = true;
        } else {
          if (!this->createSettingsTable(&db))
            recreateDb = true;
        }
      } else {
        recreateDb = true;
      }

      if (recreateDb && !this->recreateDb(&db))
        return false;

      Settings::setConnection(db.connectionName());
      if (this->isSettingsDatabaseDamaged()) {
        CRITICAL_LOG << "Unknown error after recreating settings db.";
        return false;
      }

      return true;
    }

    bool InitializeHelper::isSettingsDatabaseDamaged()
    {
      Settings settings;
      qint64 someValue = QDateTime::currentMSecsSinceEpoch();
      settings.setValue("SelfCheckValue", someValue, true);
      bool ok = false;
      qint64 result = settings.value("SelfCheckValue", -1).toLongLong(&ok);
      return !ok || someValue != result;
    }

    bool InitializeHelper::recreateDb(QSqlDatabase *db)
    {
      db->close();
      QFile dbFile(this->_fileName);
      if (dbFile.exists() && !dbFile.remove()) {
        CRITICAL_LOG << "Couldn't remove settings file. May be it's locked.";
        return false;
      }

      if (!db->open(this->_userName, this->_password)) {
        CRITICAL_LOG << "Couldn't reopen settings. " 
          << (db->lastError().isValid() ? db->lastError().text() : "Unknown error");
        return false;
      }

      if (!this->createSettingsTable(db)) {
        return false;
      }

      return true;
    }

    bool InitializeHelper::createSettingsTable(QSqlDatabase *db)
    {
      QSqlQuery query = db->exec(
        "CREATE TABLE app_settings "
        "( "
        "	key_column text NOT NULL, "
        "	value_column text, "
        "	CONSTRAINT app_settings_pk PRIMARY KEY (key_column) "
        ")");

      if (query.lastError().isValid()) {
        CRITICAL_LOG << "Couldn't create settings table. " << query.lastError().text();	
        return false;
      }

      this->_recreate = true;
      return true;
    }
  }
}