#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QDataStream>

class SerializeTestClass : public QObject 
{
  Q_OBJECT
public:
  explicit SerializeTestClass(QObject *parent = 0);
  SerializeTestClass(const SerializeTestClass &p);
  ~SerializeTestClass();

  QString str;
  int someInt;
  qreal someReal;
};

Q_DECLARE_METATYPE(SerializeTestClass);
Q_DECLARE_METATYPE(SerializeTestClass *);

QDataStream& operator<<( QDataStream& stream, const SerializeTestClass& c);
QDataStream& operator>>( QDataStream& stream, SerializeTestClass& c);
