#ifndef QTSQLSETTING_P_H
#define QTSQLSETTING_P_H

#include <QtCore/QMutex>

#include <QtCore/QStringList>
#include <QtCore/QStack>

#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>

#ifndef QT_NO_GEOM_VARIANT
#include <QtCore/QRect>
#endif  //#ifndef QT_NO_GEOM_VARIANT

class QSettingsGroup
{
public:
  inline QSettingsGroup()
    : num(-1), maxNum(-1) {}
  inline QSettingsGroup(const QString &s)
    : str(s), num(-1), maxNum(-1) {}
  inline QSettingsGroup(const QString &s, bool guessArraySize)
    : str(s), num(0), maxNum(guessArraySize ? 0 : -1) {}

  inline QString name() const { return str; }
  inline QString toString() const;
  inline bool isArray() const { return num != -1; }
  inline int arraySizeGuess() const { return maxNum; }
  inline void setArrayIndex(int i)
  { num = i + 1; if (maxNum != -1 && num > maxNum) maxNum = num; }

  QString str;
  int num;
  int maxNum;
};

inline QString QSettingsGroup::toString() const
{
  QString result;
  result = str;
  if (num > 0) {
    result += QLatin1Char('/');
    result += QString::number(num);
  }
  return result;
}

class SettingsPrivate
{
public:
  SettingsPrivate() {}
  virtual ~SettingsPrivate(){};

  typedef QHash<QString, QVariant> SettingsMap;

  SettingsMap map;

  static QString table;
  static QString connection;

  static QString keyColumn;
  static QString valueColumn;

  enum ChildSpec { AllKeys, ChildKeys, ChildGroups };
  virtual QStringList children(const QString &prefix, ChildSpec spec) const;

  QString actualKey(const QString &key) const;
  void beginGroupOrArray(const QSettingsGroup &group);

  QString normalizedKey(const QString &key) const;

  void processChild(QString key, ChildSpec spec, QMap<QString, QString> &result) const;

  QStringList variantListToStringList(const QVariantList &l) const;
  QVariant stringListToVariantList(const QStringList &l) const;

  // parser functions
  QString variantToString(const QVariant &v) const;
  QVariant stringToVariant(const QString &s) const;
  QStringList splitArgs(const QString &s, int idx) const;
  QStack<QSettingsGroup> groupStack;
  QString groupPrefix;

protected:
  mutable QMutex mutex;
};

#endif //#ifndef QTSQLSETTING_P_H
