#include "SerializeTestClass.h"


SerializeTestClass::SerializeTestClass(QObject *parent)
  : QObject(parent)
{
}

SerializeTestClass::SerializeTestClass(const SerializeTestClass &p)
  : str(p.str)
  , someInt(p.someInt)
  , someReal(p.someReal)
{

}


SerializeTestClass::~SerializeTestClass()
{
}

QDataStream& operator<<( QDataStream& stream, const SerializeTestClass& c)
{
  return stream << c.str << c.someInt << c.someReal;
}

QDataStream& operator>>( QDataStream& stream, SerializeTestClass& c) 
{
  stream >> c.str >> c.someInt >> c.someReal;
  return stream;
}