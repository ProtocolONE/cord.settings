#include <Windows.h>
//#include <vld.h>

#include <QtCore/QCoreApplication>
#include <qsqldatabase.h>
#include <qstringlist.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qdebug.h>
#include <qdatetime.h>
#include <qelapsedtimer.h>
#include <qwaitcondition.h>
#include <QtConcurrentRun>
#include <qmutex.h>
#include <qtimer.h>
#include "Settings.h"
#include "SettingsSaver.h"
#include "SettingsTestHelper.h"

#include <gtest/gtest.h>


void mstressTest(Settings* settings)
{
  settings->setValue("abcd1",123);
  settings->setValue("New Value", QStringList() << "First line" << "SecondLine");
  settings->setValue("group1/value1", QVariantList() << 1 << 3);
  settings->setValue("bytearray", QByteArray("bytearray", 9));

  for (int i=0; i < 10; ++i){
    if (settings->setValue("new value " + QString::number(i), i));
  }
}

void mAsyncstressTest(Settings* settings)
{
  settings->setValue("abcd1",123);
  settings->setValue("New Value", QStringList() << "First line" << "SecondLine");
  settings->setValue("group1/value1", QVariantList() << 1 << 3);
  settings->setValue("bytearray", QByteArray("bytearray", 9));

  for (int i=0; i < 10000; ++i){
    if (settings->setValue("new value " + QString::number(i), i, false));
  }
}

bool checkStressTest(Settings* settings, int count)
{
  bool testResult = false;
  for (int i=0; i < count; ++i){
    int _t = settings->value("new value " + QString::number(i)).toInt();

    if (i != _t)
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

int runTests()
{
 int result = RUN_ALL_TESTS();
 QCoreApplication::quit();
 return result;
}

TEST(threadedStressTest,threadedStressTest)
{
  Settings* settings = new Settings();

  settings->clear();
  ASSERT_FALSE( settings->allKeys().count() );

  for (int i = 0; i < 1; ++i)
  {
    QFuture<void> future = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future2 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future3 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future4 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future5 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future6 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future7 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future8 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future9 = QtConcurrent::run(&mstressTest, settings);
    QFuture<void> future10 = QtConcurrent::run(&mstressTest, settings);

    future.waitForFinished();
    future2.waitForFinished();
    future3.waitForFinished();
    future4.waitForFinished();
    future5.waitForFinished();
    future6.waitForFinished();
    future7.waitForFinished();
    future8.waitForFinished();
    future9.waitForFinished();
    future10.waitForFinished();

    ASSERT_FALSE(checkStressTest(settings, 10));
  }
  delete settings;
}

TEST(threadedStressTest,asyncThreadedStressTest)
{
  Settings* settings = new Settings();

  settings->clear();
  ASSERT_FALSE( settings->allKeys().count() );

  for (int i = 0; i < 1; ++i)
  {
    QFuture<void> future = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future2 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future3 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future4 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future5 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future6 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future7 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future8 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future9 = QtConcurrent::run(&mAsyncstressTest, settings);
    QFuture<void> future10 = QtConcurrent::run(&mAsyncstressTest, settings);

    future.waitForFinished();
    future2.waitForFinished();
    future3.waitForFinished();
    future4.waitForFinished();
    future5.waitForFinished();
    future6.waitForFinished();
    future7.waitForFinished();
    future8.waitForFinished();
    future9.waitForFinished();
    future10.waitForFinished();

    ASSERT_FALSE(checkStressTest(settings, 1000));
  }
  delete settings;
}

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  testing::InitGoogleTest(&argc, argv);
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

  Settings::setConnection(db.connectionName());
  Settings::setSettingsSaver( new SettingsSaver() );

  QFuture<int> testResult = QtConcurrent::run(runTests);
  a.exec();

  return testResult.result();
}
