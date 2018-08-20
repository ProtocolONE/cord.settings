#pragma once

#include <QtCore/qglobal.h>

#ifdef SETTINGS_STATIC_LIB
  # define SETTINGSLIB_EXPORT  
#else

#ifdef SETTINGS_LIB
# define SETTINGSLIB_EXPORT Q_DECL_EXPORT
#else
# define SETTINGSLIB_EXPORT Q_DECL_IMPORT
#endif

#endif

#define CRITICAL_LOG qCritical() << __FILE__ << __LINE__ << __FUNCTION__
#define WARNING_LOG qWarning() << __FILE__ << __LINE__ << __FUNCTION__
#define DEBUG_LOG qDebug() << __FILE__ << __LINE__ << __FUNCTION__
