#include <Settings/Settings.h>
#include <Settings/Settings_p.h>
#include <Settings/SettingsSaver.h>

#include <QtCore/QFuture>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>

#ifndef QT_NO_GEOM_VARIANT
#include <QtCore/QRect>
#endif  //#ifndef QT_NO_GEOM_VARIANT

namespace GGS {
  namespace Settings {

    QString Settings::_deleteQueryTemplate;
    QString Settings::_removeQueryTemplate;
    QString Settings::_replaceQueryTemplate;
    QString Settings::_selectQueryTemplate;
    bool Settings::isBeginTransaction = false;
    SettingsSaver* Settings::_settingsSaver;

    bool Settings::_isInitialized = false;

    QMutex Settings::lockMutex;

    Settings::Settings(QObject *parent)
      : QObject(parent),
        _settingsPrivate(new SettingsPrivate())
    {
      QSqlDatabase db = QSqlDatabase::database(this->_settingsPrivate->connection);

      this->_deleteQueryTemplate = QString("DELETE FROM %1").arg( db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName));
      _removeQueryTemplate = QString("DELETE FROM %1 WHERE %2==?").arg( db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName), 
        db.driver()->escapeIdentifier(SettingsPrivate::keyColumn, QSqlDriver::FieldName));

      this->_replaceQueryTemplate = QString("REPLACE INTO %1(%2, %3) VALUES (?, ?)").arg(db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName)
        , db.driver()->escapeIdentifier(SettingsPrivate::keyColumn, QSqlDriver::FieldName)
        , db.driver()->escapeIdentifier(SettingsPrivate::valueColumn, QSqlDriver::FieldName));

      this->_selectQueryTemplate = QString("SELECT %2,%3 FROM %1 WHERE %2==?").arg( db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName)
        ,db.driver()->escapeIdentifier(SettingsPrivate::keyColumn, QSqlDriver::FieldName)
        ,db.driver()->escapeIdentifier(SettingsPrivate::valueColumn, QSqlDriver::FieldName));
    }

    Settings::~Settings() {
    }

    void Settings::sync()
    {
      if (!isBeginTransaction)
        return;

      QMutexLocker locker(&lockMutex);

      QSqlDatabase db = QSqlDatabase::database(SettingsPrivate::connection);
      db.driver()->commitTransaction();
      isBeginTransaction = false;
    }

    QStringList Settings::allKeys() const
    {
      return this->_settingsPrivate->children(this->_settingsPrivate->groupPrefix, SettingsPrivate::AllKeys);
    }

    void Settings::beginGroup(const QString &prefix)
    {
      this->_settingsPrivate->beginGroupOrArray(QSettingsGroup(this->_settingsPrivate->normalizedKey(prefix)));
    }

    int Settings::beginReadArray(const QString &prefix)
    {
      this->_settingsPrivate->beginGroupOrArray(QSettingsGroup(this->_settingsPrivate->normalizedKey(prefix), false));
      return value(QLatin1String("size")).toInt();
    }

    void Settings::beginWriteArray(const QString &prefix, int size)
    {
      this->_settingsPrivate->beginGroupOrArray(QSettingsGroup(this->_settingsPrivate->normalizedKey(prefix), size < 0));

      if (size < 0)
        remove(QLatin1String("size"));
      else
        setValue(QLatin1String("size"), size);
    }

    QStringList Settings::childGroups() const
    {
      return this->_settingsPrivate->children(this->_settingsPrivate->groupPrefix, SettingsPrivate::ChildGroups);
    }

    QStringList Settings::childKeys() const
    {
      return this->_settingsPrivate->children(this->_settingsPrivate->groupPrefix, SettingsPrivate::ChildKeys);
    }

    bool Settings::clear()
    {
      Q_ASSERT(!SettingsPrivate::connection.isEmpty());
      QSqlDatabase db = QSqlDatabase::database(this->_settingsPrivate->connection);
      QSqlQuery sqlQuery(db);

      if (!(sqlQuery.exec(deleteQueryTemplate())))
      {
        qWarning() << Q_FUNC_INFO;
        qWarning() << sqlQuery.lastError().text();
        return true;
      } 

      return false;
    }

    bool Settings::contains(const QString &key) const
    {
      QString k = this->_settingsPrivate->actualKey(key);
      return value(key) != QVariant();
    }

    void Settings::endArray()
    {
      if (this->_settingsPrivate->groupStack.isEmpty()) {
        qWarning("QSettings::endArray: No matching beginArray()");
        return;
      }

      QSettingsGroup group = this->_settingsPrivate->groupStack.top();
      int len = group.toString().size();
      this->_settingsPrivate->groupStack.pop();
      if (len > 0)
        this->_settingsPrivate->groupPrefix.truncate(this->_settingsPrivate->groupPrefix.size() - (len + 1));

      if (group.arraySizeGuess() != -1)
        setValue(group.name() + QLatin1String("/size"), group.arraySizeGuess());

      if (!group.isArray())
        qWarning("QSettings::endArray: Expected endGroup() instead");
    }

    void Settings::endGroup()
    {
      if (this->_settingsPrivate->groupStack.isEmpty()) {
        qWarning("QSettings::endGroup: No matching beginGroup()");
        return;
      }

      QSettingsGroup group = this->_settingsPrivate->groupStack.pop();
      int len = group.toString().size();
      if (len > 0)
        this->_settingsPrivate->groupPrefix.truncate(this->_settingsPrivate->groupPrefix.size() - (len + 1));

      if (group.isArray())
        qWarning("QSettings::endGroup: Expected endArray() instead");
    }

    QString Settings::group() const
    {
      return this->_settingsPrivate->groupPrefix.left(this->_settingsPrivate->groupPrefix.size() - 1);
    }

    bool Settings::remove(const QString &key)
    {
      //            QMutexLocker locker(&mutex);
      /*
      We cannot use actualKey(), because remove() supports empty
      keys. The code is also tricky because of slash handling.
      */
      Q_ASSERT(!SettingsPrivate::connection.isEmpty());

      QString theKey = this->_settingsPrivate->normalizedKey(key);
      if (theKey.isEmpty())
        theKey = group();
      else
        theKey.prepend(this->_settingsPrivate->groupPrefix);

      if (theKey.size()) {
        QSqlDatabase db = QSqlDatabase::database(this->_settingsPrivate->connection);
        QSqlQuery sqlQuery(db);
        sqlQuery.prepare(removeQueryTemplate());
        sqlQuery.addBindValue(key);

        if (!(sqlQuery.exec()))
        {
          qWarning() << Q_FUNC_INFO;
          qWarning() << sqlQuery.lastError().text();
          qWarning() << sqlQuery.lastError().type();

          return true;
        }

        return false;
      }

      return false;
    }

    void Settings::setArrayIndex(int i)
    {
      if (this->_settingsPrivate->groupStack.isEmpty() || !this->_settingsPrivate->groupStack.top().isArray()) {
        qWarning("QSettings::setArrayIndex: Missing beginArray()");
        return;
      }

      QSettingsGroup &top = this->_settingsPrivate->groupStack.top();
      int len = top.toString().size();
      top.setArrayIndex(qMax(i, 0));
      this->_settingsPrivate->groupPrefix.replace(this->_settingsPrivate->groupPrefix.size() - len - 1, len, top.toString());
    }

    /*
    Внимание! Операция занимает от 80мс в режиме мгновенной записи, подробности
    http://www.sqlite.org/faq.html#q19
    */
    bool Settings::setValue(const QString &key, const QVariant &value, bool isInstantlySave)
    {
      Q_ASSERT(!SettingsPrivate::connection.isEmpty());

      QString k = this->_settingsPrivate->actualKey(key);

      QSqlDatabase db = QSqlDatabase::database(this->_settingsPrivate->connection);
      QSqlQuery sqlQuery(db);

      if (!isInstantlySave && !isBeginTransaction)
      {
        db.driver()->beginTransaction();
        isBeginTransaction = true;
      }
      if (isInstantlySave && isBeginTransaction)
        Settings::sync();

      sqlQuery.prepare(replaceQueryTemplate());
      sqlQuery.addBindValue(k);
      sqlQuery.addBindValue(this->_settingsPrivate->variantToString(value));

      if (!(sqlQuery.exec( )))
      {
        qWarning() << sqlQuery.lastError().text();
        return true;
      } 

      return false;
    }

    QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
    {
      Q_ASSERT(!SettingsPrivate::connection.isEmpty());
      QString k = this->_settingsPrivate->actualKey(key);

      QSqlDatabase db = QSqlDatabase::database(this->_settingsPrivate->connection);
      QSqlQuery sqlQuery(db);

      sqlQuery.prepare(selectQueryTemplate());
      sqlQuery.addBindValue(k);

      if (!(sqlQuery.exec( )))
      {
        qWarning() << Q_FUNC_INFO;
        qWarning() << sqlQuery.lastError().text();

        return defaultValue;
      }  
      if (sqlQuery.first())
        return this->_settingsPrivate->stringToVariant(sqlQuery.value(1).toString());

      return defaultValue;
    }

    QString Settings::table() const
    {
      return this->_settingsPrivate->table;
    }

    QString Settings::connection() const
    {
      return this->_settingsPrivate->connection;
    }

    void Settings::setTable(const QString &table)
    {
      SettingsPrivate::table = table;
    }

    void Settings::setConnection(const QString& connection)
    {
      SettingsPrivate::setConnection(connection);
      _isInitialized = true;
    }

    QString Settings::keyColumn() const
    {
      return this->_settingsPrivate->keyColumn;
    }

    QString Settings::valueColumn() const
    {
      return this->_settingsPrivate->valueColumn;
    }

    void Settings::setKeyColumn(const QString &columnName)
    {
      SettingsPrivate::keyColumn = columnName;
    }

    void Settings::setValueColumn(const QString &columnName)
    {
      SettingsPrivate::valueColumn = columnName;
    }

    QString Settings::deleteQueryTemplate()
    {
      return _deleteQueryTemplate;
    }

    QString Settings::removeQueryTemplate()
    {
      return _removeQueryTemplate;
    }

    QString Settings::replaceQueryTemplate()
    {
      return _replaceQueryTemplate;
    }

    QString Settings::selectQueryTemplate()
    {
      return _selectQueryTemplate;
    }

    void Settings::setSettingsSaver(SettingsSaver* settingsSaver)
    {
      Q_CHECK_PTR(settingsSaver);
      _settingsSaver = settingsSaver;
    }

    bool Settings::isInitialized()
    {
      return _isInitialized;
    }

  }
}