#include <Settings/Settings.h>
#include <Settings/SettingsSaver.h>
#include <Settings/InitializeHelper.h>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtConcurrentRun>

#include <gtest/gtest.h>

#include "SerializeTestClass.h"

using namespace GGS::Settings;

int run()
{ 
  int result = RUN_ALL_TESTS();
  
  QCoreApplication::quit();
  return result;
}

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  QStringList plugins;
  QString path = QCoreApplication::applicationDirPath();

  plugins << path + "/plugins";
  a.setLibraryPaths(plugins);

  testing::InitGoogleTest(&argc, argv);

  qRegisterMetaType<SerializeTestClass>("SerializeTestClass");

  //Use default helper settings
  InitializeHelper helper;
  if (!helper.init()) 
    return -1;
 
  QScopedPointer<SettingsSaver> saver(new SettingsSaver());
  Settings::setSettingsSaver(saver.data());
  
  //«апуск через отдельный поток сделать специально - это обеспечивает
  //работу EventLoop`а дл€ SettingsSaver.
  QFuture<int> result = QtConcurrent::run(&run);
  result.waitForFinished();

  return result.result();
}
