#include <gtest/gtest.h>

#include <Settings/InitializeHelper.h>

#include <QtCore/QFile>
#include <QtCore/QCoreApplication>

#include <QtSql/QSqlDatabase>

using namespace GGS::Settings;

TEST(InitializeHelperTest, defaultSettingsTest)
{
  InitializeHelper helper;
  ASSERT_NE("", helper.userName());
  ASSERT_NE("", helper.password());
  ASSERT_NE("", helper.fileName());
  ASSERT_NE("", helper.connectionName());
}

TEST(InitializeHelperTest, setterGetterTest)
{
  InitializeHelper helper;
  helper.setUserName(QString("name"));
  ASSERT_EQ("name", helper.userName());

  helper.setPassword(QString("pass"));
  ASSERT_EQ("pass", helper.password());

  helper.setFileName(QString("filename"));
  ASSERT_EQ("filename", helper.fileName());

  helper.setConnectionName(QString("connection"));
  ASSERT_EQ("connection", helper.connectionName());
}

TEST(InitializeHelperTest, successOpenTest)
{
  QFile file(QCoreApplication::applicationDirPath() + "/successOpenTest.sql");
  file.remove();

  InitializeHelper helper;
  helper.setConnectionName(QString("testDbConnection"));
  helper.setFileName(file.fileName());
  helper.setUserName(QString("user"));
  helper.setPassword(QString("pass"));

  ASSERT_TRUE(helper.init());
  ASSERT_TRUE(helper.isRecreated());
}

TEST(InitializeHelperTest, openDbWithBadPathTest)
{
  InitializeHelper helper;
  helper.setConnectionName(QString("openDbWithBadPathTest"));
  helper.setFileName(QString("some\\bad\\path\\to\\file"));

  ASSERT_FALSE(helper.init());
  ASSERT_FALSE(helper.isRecreated());
}

TEST(InitializeHelperTest, openDbWithEmptyDbFileTest)
{
  QFile file(QCoreApplication::applicationDirPath() + "/openDbWithEmptyDbFileTest.sql");
  file.remove();
  file.open(QFile::Truncate | QFile::ReadWrite);
  file.close();

  InitializeHelper helper;
  helper.setConnectionName(QString("openDbWithEmptyDbFileTest"));
  helper.setFileName(file.fileName());

  ASSERT_TRUE(helper.init());
  ASSERT_TRUE(helper.isRecreated());
}

TEST(InitializeHelperTest, openDbWithDamagedDbTest)
{
  QFile file(QCoreApplication::applicationDirPath() + "/openDbWithDamagedDbTest.sql");
  file.remove();
  file.open(QFile::Truncate | QFile::ReadWrite);
  file.write("trash\0");
  file.close();
  
  InitializeHelper helper;
  helper.setConnectionName(QString("openDbWithDamagedDbTest"));
  helper.setFileName(file.fileName());

  ASSERT_TRUE(helper.init());
  ASSERT_TRUE(helper.isRecreated());
}