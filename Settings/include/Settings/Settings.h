/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (В©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_SETTINGS_SETTING_H
#define _GGS_SETTINGS_SETTING_H

#include <Settings/settings_global.h>
#include <Settings/Settings_p.h>
#include <Settings/SettingsSaver.h>

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMutex>

/*!
  The Settings class provides persistent platform-independent application settings. Settings are stored in database.
  This class is not a descendant of QSettings, but it provides an identical interface.
  All functions in this class are reentrant.

  Using in your project:
  
  Create qt sql connection, for e.g:

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("database");
  if (db.open("admin", "admin")) {

  if (!db.tables().contains("app_settings")) {
  QSqlQuery q = db.exec("CREATE TABLE app_settings "
  "( "
  "	key_column text NOT NULL, "
  "	value_column text, "
  "	CONSTRAINT app_settings_pk PRIMARY KEY (key_column) "
  ")");
  if (q.lastError().isValid())
  qDebug() << q.lastError().text();
  }
  }

  Create Settings object with opened connection name and load settings from table, for e.g:

  #include <Settings/Settings.h>

  Settings settings;
  Settings::setConnection(db.connectionName());
  Settings::setSettingsSaver( new SettingsSaver() ); 

  // Задание SettingsSaver необходимо для сохранения ключей которые сохраняются через 1 сек. интервалы,
  // подробности ниже.

  Read|write settings as if the object is QSettings, for e.g:

  settings->setValue("New Value", QStringList() << "First line" << "SecondLine");
  settings->setValue("group1/value1", QVariantList() << 1 << 3);
  settings->setValue("bytearray", QByteArray("bytearray", 9));


  Метод setValue последним параметром принимает значение, которое указывает на необходимость принудительно записи
  в базу (true), или (false) при которой ключи будут сгруппированы в течении 1 секунды в одну транзакцию, и сохранятся
  намного быстрее чем при мгновенной записи.

  Некоторые оптимизации. 

  Если вам необходимо сохранять сразу несколько ключей, никогда не используйте мгновенную запись!
  Это операция может занять довольно большйо промежуток времени. Одна запись может длится от 80 до 300 мс. Почему это так
  можно почитать здесь http://www.sqlite.org/faq.html#q19.

  При записи множества значений сразу, используйте следующий код
  settings->setValue("a1", 1, false);
  settings->setValue("a2", 1, false);
  settings->setValue("a3", 1, false);
  settings->setValue("a4", 1, false);
  ...
  settings->setValue("a10", 10, true); // обратите внимание на последний true!

  При таком подходе, вышеперечисленные ключи будут сохранены в одно транзакции, что в РАЗЫ значительно ускорит сохранение.
  Последний параметр вы задали как true, для того что-бы все наши значения сохранились сразу, а не ожидали еще 1 секунду в
  очереди на запись.

  Методы setValue/value поддерживают кеширование. Включить его можно через функцию setCacheEnabled.

  @class Settings Settings.h
*/
namespace GGS {
  namespace Settings {
    //class SettingsPrivate;
    //class SettingsSaver;

    class SETTINGSLIB_EXPORT Settings : public QObject
    {
      Q_OBJECT

      Q_PROPERTY(QString table READ table WRITE setTable)
      Q_PROPERTY(QString connection READ connection WRITE setConnection)
      Q_PROPERTY(QString keyColumn READ keyColumn WRITE setKeyColumn)
      Q_PROPERTY(QString valueColumn READ valueColumn WRITE setValueColumn)

    private:
      QScopedPointer<SettingsPrivate> _settingsPrivate;

    public:
      explicit Settings(QObject* parent = 0);
      virtual ~Settings();

      QString table() const;
      QString connection() const;
      QString keyColumn() const;
      QString valueColumn() const;

      QStringList allKeys() const;

      void  beginGroup(const QString& prefix);
      int beginReadArray(const QString& prefix);
      void  beginWriteArray(const QString& prefix, int size = -1);
      QStringList childGroups() const;
      QStringList childKeys() const;
      bool  clear();
      bool  contains(const QString& key) const;
      void  endArray();
      void  endGroup();
      QString group() const;
      bool  remove(const QString& key);
      void  setArrayIndex(int i);

      bool  setValue(const QString& key, const QVariant& value, bool isInstantlySave = true);
      QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

      static QString deleteQueryTemplate(); 
      static QString removeQueryTemplate(); 
      static QString replaceQueryTemplate(); 
      static QString selectQueryTemplate();

    public slots:
      static void sync();

    public:
      static void setTable(const QString& table);
      static void setConnection(const QString& connection);

      static void setKeyColumn(const QString& columnName);
      static void setValueColumn(const QString& columnName);

      static void setSettingsSaver(SettingsSaver* settingsSaver); 

      static bool isInitialized();
      static void setCacheEnabled(bool enabled);

    private:
      static QString _deleteQueryTemplate;
      static QString _removeQueryTemplate;
      static QString _replaceQueryTemplate;
      static QString _selectQueryTemplate;

      mutable QMutex mutex;
      static QMutex lockMutex;
      static bool isBeginTransaction;
      static SettingsSaver* _settingsSaver;

      static bool _isInitialized;

      static bool _isCacheEnabled;
      static QMutex _cacheMutex;
      static QHash<QString, QVariant> _cache;
      static bool tryGetFromCache(const QString& normalizedKey, QVariant& result);
      static void putToCache(const QString& normalizedKey, const QVariant& value);
    };
  }
}
#endif // _GGS_SETTINGS_SETTING_H
