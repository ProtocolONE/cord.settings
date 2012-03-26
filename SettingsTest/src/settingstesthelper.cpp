#include "settingstesthelper.h"
#include "gtest/gtest.h"

SettingsTestHelper::SettingsTestHelper(QObject *parent)
    : QObject(parent)
{

}

SettingsTestHelper::~SettingsTestHelper()
{

}

void SettingsTestHelper::runTest()
{
    RUN_ALL_TESTS();
}