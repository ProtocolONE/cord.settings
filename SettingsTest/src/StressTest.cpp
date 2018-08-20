#include <gtest/gtest.h>

#include <Settings/Settings.h>
#include <Settings/SettingsSaver.h>

#include <QtConcurrentRun>

using namespace P1::Settings;

class StressTest : public ::testing::Test
{
public:
  Settings settings;

  void SetUp()
  {
    this->settings.clear();
    ASSERT_EQ(0, this->settings.allKeys().count());
  }

  void testLoopWith10Thread(quint32 count = 10, bool isInstantlySave = true)
  {
    QFuture<void> future = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future2 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future3 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future4 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future5 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future6 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future7 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future8 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future9 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);
    QFuture<void> future10 = QtConcurrent::run(this, &StressTest::testLoop, count, isInstantlySave);

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
  }

  void testLoop(quint32 count = 10, bool isInstantlySave = true)
  {
    settings.setValue("abcd1",123);
    settings.setValue("New Value", QStringList() << "First line" << "SecondLine");
    settings.setValue("group1/value1", QVariantList() << 1 << 3);
    settings.setValue("bytearray", QByteArray("bytearray", 9));

    for (int i = 0; i < count; ++i)
      settings.setValue("new value " + QString::number(i), i, isInstantlySave);
  }

  void testIsDataValidForTestLoop(int count)
  {
    for (int i = 0; i < count; ++i) 
      ASSERT_EQ(i, settings.value("new value " + QString::number(i)).toInt());
    
    ASSERT_EQ(123, settings.value("abcd1").toInt());

    QStringList str = settings.value("New Value").toStringList();
    ASSERT_EQ("First line", str.value(0));
    ASSERT_EQ(QString("SecondLine"), str.value(1));

    QList<QVariant> vlist = settings.value("group1/value1").toList();
    ASSERT_EQ(1, vlist.value(0).toInt());
    ASSERT_EQ(3, vlist.value(1).toInt());

    QByteArray barray = settings.value("bytearray").toByteArray();
    ASSERT_EQ(9, barray.count());
  }
};

