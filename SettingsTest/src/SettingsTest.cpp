#include <Settings/Settings.h>
#include <Settings/SettingsSaver.h>
#include <Settings/InitializeHelper.h>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QElapsedTimer>
#include <QtCore/QDate>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QtConcurrentRun>
#include <gtest/gtest.h>

#include "SerializeTestClass.h"

using namespace GGS::Settings;

TEST(serializeTests, toQByteArray)
{
  QHash<QString, SerializeTestClass> someMap;
  someMap["test1"].someInt = 12;
  someMap["test1"].someReal = 13.3f;
  someMap["test1"].str = "str1";
  someMap["test2"].someInt = 22;
  someMap["test2"].someReal = 23.3f;
  someMap["test2"].str = "str2";

  QByteArray ba;
  QDataStream out1(&ba, QIODevice::WriteOnly);
  out1.setVersion(QDataStream::Qt_4_7);
  out1 << someMap;

  Settings settings;
  settings.beginGroup("serializeTests");
  settings.setValue("key", ba);

  QByteArray baResult = settings.value("key", QByteArray()).toByteArray();
  QDataStream in1(&baResult, QIODevice::ReadOnly);
  in1.setVersion(QDataStream::Qt_4_7);
  QHash<QString, SerializeTestClass> someMap2;
  in1 >> someMap2;
}

TEST(stressTest, stressTest)
{
  bool testResult = false;

  QVector<int> testList;
  Settings* settings = new Settings();
  settings->beginGroup("my_group");

  settings->setValue("abcd1",123);
  settings->setValue("New Value", QStringList() << "First line" << "SecondLine");
  settings->setValue("group1/value1", QVariantList() << 1 << 3);
  settings->setValue("bytearray", QByteArray("bytearray", 9));

  int i;
  for (i=0; i < 20; ++i){
    int _t = rand() % 100;
    if (settings->setValue("new value " + QString::number(i), _t))
      testResult = true;
    testList.push_back(_t);
  }

  for (i=0; i < 20; ++i){
    int _t = settings->value("new value " + QString::number(i)).toInt();

    if (testList[i] != _t)
      testResult = true;
  }

  if (settings->value("abcd1").toInt() != 123)
    testResult = true;

  QStringList str = settings->value("New Value").toStringList();
  if (str.value(0) != "First line" || str.value(1) != "SecondLine")
    testResult = true;

  QList<QVariant> vlist = settings->value("group1/value1").toList();
  if (vlist.value(0).toInt() != 1 || vlist.value(1).toInt() != 3)
    testResult = true;

  QByteArray barray = settings->value("bytearray").toByteArray();
  if (barray.count() != 9)
    testResult = true;
  delete settings;

  ASSERT_FALSE(testResult);
}

bool stressTest(Settings* settings)
{
  bool testResult = false;

  QVector<int> testList;
  settings->beginGroup("my_group");

  settings->setValue("abcd1",123);
  settings->setValue("New Value", QStringList() << "First line" << "SecondLine");
  settings->setValue("group1/value1", QVariantList() << 1 << 3);
  settings->setValue("bytearray", QByteArray("bytearray", 9));

  int i;
  for (i=0; i < 20; ++i){
    int _t = rand() % 100;
    if (settings->setValue("new value " + QString::number(i), _t))
      testResult = true;

    testList.push_back(_t);
  }

  for (i=0; i < 20; ++i){
    int _t = settings->value("new value " + QString::number(i)).toInt();

    if (testList[i] != _t)
      testResult = true;
  }

  if (settings->value("abcd1").toInt() != 123)
    testResult = true;

  QStringList str = settings->value("New Value").toStringList();
  if (str.value(0) != "First line" || str.value(1) != "SecondLine")
    testResult = true;

  QList<QVariant> vlist = settings->value("group1/value1").toList();
  if (vlist.value(0).toInt() != 1 || vlist.value(1).toInt() != 3)
    testResult = true;

  QByteArray barray = settings->value("bytearray").toByteArray();
  if (barray.count() != 9)
    testResult = true;

  return testResult;
}

TEST(arrayTest,arrayTest)
{
  bool testResult = false;

  Settings *settings = new Settings();
  QVector<int> testList;

  settings->beginWriteArray("my_array");
  int i;
  for (i=0; i < 15; ++i){
    int _t = rand() % 100;
    testList.push_back(_t);
    settings->setArrayIndex(i);
    if (settings->setValue("new value " + QString::number(i), _t))
      testResult = true;
  }
  settings->endArray();

  settings->beginReadArray("my_array");
  for (i=0; i < 15; ++i){
    settings->setArrayIndex(i);
    int _t = settings->value("new value " + QString::number(i)).toInt();
    if (testList[i] != _t)
      testResult = true;
  }
  settings->endArray();

  delete settings;

  ASSERT_FALSE(testResult);
}

TEST(groupTest, groupTest)
{
  Settings *settings = new Settings();

  QElapsedTimer timer;

  settings->beginGroup("my_group");

  ASSERT_FALSE(stressTest(settings));

  settings->endGroup();

  delete settings;

}

TEST(syncSetterGetterTest,syncSetterGetterTest)
{
  Settings *settings = new Settings();

  ASSERT_FALSE(stressTest(settings));

  delete settings;
}

TEST(asyncSetterGetterTest,asyncSetterGetterTest2)
{
  bool testResult = false;

  Settings *settings = new Settings();

  QVector<int> testList;

  int i;
  for (i=0; i < 25; ++i){
    int _t = rand() % 100;
    ASSERT_FALSE(settings->setValue("new value " + QString::number(i), _t, false));

    testList.push_back(_t);
  }

  delete settings;

  Settings *settings2 = new Settings();

  for (i=0; i < 25; ++i){
    int _t = settings2->value("new value " + QString::number(i)).toInt();

    ASSERT_EQ(testList[i],_t);
  }

  delete settings2;
}

TEST(keysTest,keysTest)
{
  Settings* settings = new Settings();

  bool testResult = false;

  QStringList list = settings->allKeys();

  foreach(QString key, list)
  {
    if (!settings->contains(key))
      testResult = true;
  }

  settings->beginGroup("my_group");
  list = settings->allKeys();

  foreach(QString key, list)
    ASSERT_TRUE(settings->contains(key));

  settings->endGroup();

  settings->beginReadArray("my_array");
  settings->setArrayIndex(0);
  list = settings->allKeys();
  foreach(QString key, list)
  {
    ASSERT_TRUE(settings->contains(key));
  }
  settings->endArray();

  delete settings;
}

TEST(removeTest,removeTest)
{
  Settings* settings = new Settings();
  settings->clear();

  ASSERT_FALSE(settings->allKeys().count());

  delete settings;
}

TEST(syncAsyncTest, syncAsyncTest)
{
  Settings* settings = new Settings();
  settings->clear();

  ASSERT_FALSE(settings->allKeys().count());

  QElapsedTimer timer;
  timer.start();
  for (int i=0; i < 500; ++i){
    int _t = settings->setValue("new value async " + QString::number(i*2), QString::number(i*2), false);
  }
  int asyncTime = timer.elapsed();

  timer.start();
  for (int i=0; i < 100; ++i){
    int _t = settings->setValue("new value sync " + QString::number(i), QString::number(i), true);
  }
  int syncTime = timer.elapsed();

  ASSERT_TRUE(syncTime > asyncTime);

  delete settings;
}

TEST(settingsCache, cacheTest) {
  Settings settings;
  QString key("cacheTest_key1");
  QDate value = QDate::currentDate();

  Settings::setCacheEnabled(true);
  ASSERT_FALSE(settings.contains(key));

  settings.setValue(key, value);
  ASSERT_EQ(value, settings.value(key, QVariant()));

  Settings::setCacheEnabled(false);
  ASSERT_EQ(value, settings.value(key, QVariant()));
}

TEST(settingsCache, cacheTest2) {
  Settings settings;
  QString key("cacheTest_key2");
  QDate value = QDate::currentDate();

  Settings::setCacheEnabled(true);
  ASSERT_FALSE(settings.contains(key));
  Settings::setCacheEnabled(false);
  ASSERT_FALSE(settings.contains(key));

  settings.setValue(key, value);
  ASSERT_EQ(value, settings.value(key, QVariant()));

  Settings::setCacheEnabled(true);
  ASSERT_EQ(value, settings.value(key, QVariant()));
}