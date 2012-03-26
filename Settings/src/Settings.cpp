#include <windows.h>
#include "Settings.h"
#include "Settings_p.h"

#include <QtCore/QDebug>

#include <QtCore/QMutexLocker>

#ifndef QT_NO_GEOM_VARIANT
#include <QtCore/QRect>
#endif  //#ifndef QT_NO_GEOM_VARIANT

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QTimerEvent>
#include <QElapsedTimer>
#include <qtconcurrentrun.h>

QString Settings::_deleteQueryTemplate;
QString Settings::_removeQueryTemplate;
QString Settings::_replaceQueryTemplate;
QString Settings::_selectQueryTemplate;
bool Settings::isBeginTransaction = false;
SettingsSaver* Settings::_settingsSaver;

QMutex Settings::lockMutex;

Settings::Settings(QObject *parent)
  :QObject(parent), d()
{
  Settings::setConnection(QSqlDatabase::defaultConnection);
  setQueryTemplate();
}

Settings::Settings(const QString &connection, QObject *parent)
  :QObject(parent), d()
{
  Settings::setConnection(connection);
  setQueryTemplate();
}

void Settings::setQueryTemplate()
{
  QMutexLocker locker(&mutex);

  QSqlDatabase db = QSqlDatabase::database(d.connection);

  _deleteQueryTemplate = QString("DELETE FROM %1").arg( db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName));
  _removeQueryTemplate = QString("DELETE FROM %1 WHERE %2==?").arg( db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName), 
    db.driver()->escapeIdentifier(SettingsPrivate::keyColumn, QSqlDriver::FieldName));

  _replaceQueryTemplate = QString("REPLACE INTO %1(%2, %3) VALUES (?, ?)").arg(db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName)
    , db.driver()->escapeIdentifier(SettingsPrivate::keyColumn, QSqlDriver::FieldName)
    , db.driver()->escapeIdentifier(SettingsPrivate::valueColumn, QSqlDriver::FieldName));

  _selectQueryTemplate = QString("SELECT %2,%3 FROM %1 WHERE %2==?").arg( db.driver()->escapeIdentifier(SettingsPrivate::table, QSqlDriver::TableName)
    ,db.driver()->escapeIdentifier(SettingsPrivate::keyColumn, QSqlDriver::FieldName)
    ,db.driver()->escapeIdentifier(SettingsPrivate::valueColumn, QSqlDriver::FieldName));
}

Settings::~Settings()
{
}

void Settings::sync()
{
 if (!isBeginTransaction)
     return;

 QMutexLocker locker(&lockMutex);

 QSqlDatabase db = QSqlDatabase::database(SettingsPrivate::connection);
 QElapsedTimer timer;
 timer.start();
 db.driver()->commitTransaction();
 isBeginTransaction = false;
}

QStringList Settings::allKeys() const
{
  return d.children(d.groupPrefix, SettingsPrivate::AllKeys);
}

void Settings::beginGroup(const QString &prefix)
{
  d.beginGroupOrArray(QSettingsGroup(d.normalizedKey(prefix)));
}

int Settings::beginReadArray(const QString &prefix)
{
  d.beginGroupOrArray(QSettingsGroup(d.normalizedKey(prefix), false));
  return value(QLatin1String("size")).toInt();
}

void Settings::beginWriteArray(const QString &prefix, int size)
{
  d.beginGroupOrArray(QSettingsGroup(d.normalizedKey(prefix), size < 0));

  if (size < 0)
    remove(QLatin1String("size"));
  else
    setValue(QLatin1String("size"), size);
}

QStringList Settings::childGroups() const
{
  return d.children(d.groupPrefix, SettingsPrivate::ChildGroups);
}

QStringList Settings::childKeys() const
{
  return d.children(d.groupPrefix, SettingsPrivate::ChildKeys);
}

bool Settings::clear()
{
  QMutexLocker locker(&mutex);
  QSqlDatabase db = QSqlDatabase::database(d.connection);
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
  QString k = d.actualKey(key);
  return value(key) != QVariant();
}

void Settings::endArray()
{
  if (d.groupStack.isEmpty()) {
    qWarning("QSettings::endArray: No matching beginArray()");
    return;
  }

  QSettingsGroup group = d.groupStack.top();
  int len = group.toString().size();
  d.groupStack.pop();
  if (len > 0)
    d.groupPrefix.truncate(d.groupPrefix.size() - (len + 1));

  if (group.arraySizeGuess() != -1)
    setValue(group.name() + QLatin1String("/size"), group.arraySizeGuess());

  if (!group.isArray())
    qWarning("QSettings::endArray: Expected endGroup() instead");
}

void Settings::endGroup()
{
  if (d.groupStack.isEmpty()) {
    qWarning("QSettings::endGroup: No matching beginGroup()");
    return;
  }

  QSettingsGroup group = d.groupStack.pop();
  int len = group.toString().size();
  if (len > 0)
    d.groupPrefix.truncate(d.groupPrefix.size() - (len + 1));

  if (group.isArray())
    qWarning("QSettings::endGroup: Expected endArray() instead");
}

QString Settings::group() const
{
  return d.groupPrefix.left(d.groupPrefix.size() - 1);
}

bool Settings::remove(const QString &key)
{
  QMutexLocker locker(&mutex);
  /*
  We cannot use actualKey(), because remove() supports empty
  keys. The code is also tricky because of slash handling.
  */
  QString theKey = d.normalizedKey(key);
  if (theKey.isEmpty())
    theKey = group();
  else
    theKey.prepend(d.groupPrefix);

  if (theKey.size()) {
    QSqlDatabase db = QSqlDatabase::database(d.connection);
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
  QMutexLocker locker(&mutex);
  if (d.groupStack.isEmpty() || !d.groupStack.top().isArray()) {
    qWarning("QSettings::setArrayIndex: Missing beginArray()");
    return;
  }

  QSettingsGroup &top = d.groupStack.top();
  int len = top.toString().size();
  top.setArrayIndex(qMax(i, 0));
  d.groupPrefix.replace(d.groupPrefix.size() - len - 1, len, top.toString());
}

/*
Внимание! Операция занимает от 80мс, подробности
http://www.sqlite.org/faq.html#q19
*/
bool Settings::setValue(const QString &key, const QVariant &value, bool isInstantlySave)
{
  QMutexLocker locker(&mutex);
  QString k = d.actualKey(key);

  QSqlDatabase db = QSqlDatabase::database(d.connection);
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
  sqlQuery.addBindValue(d.variantToString(value));

    if (!(sqlQuery.exec( )))
    {
      qWarning() << sqlQuery.lastError().text();
      return true;
    } 

  return false;
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
  QMutexLocker locker(&mutex);
  QString k = d.actualKey(key);

  QSqlDatabase db = QSqlDatabase::database(d.connection);
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
    return d.stringToVariant(sqlQuery.value(1).toString());

  return defaultValue;
}

QString Settings::table() const
{
  return d.table;
}

QString Settings::connection() const
{
  return d.connection;
}

void Settings::setTable(const QString &table)
{
  SettingsPrivate::table = table;
}

void Settings::setConnection(const QString& connection)
{
  SettingsPrivate::connection = connection;
}

QString Settings::keyColumn() const
{
  return d.keyColumn;
}

QString Settings::valueColumn() const
{
  return d.valueColumn;
}

void Settings::setKeyColumn(const QString &columnName)
{
  SettingsPrivate::keyColumn = columnName;
}

void Settings::setValueColumn(const QString &columnName)
{
  SettingsPrivate::valueColumn = columnName;
}
