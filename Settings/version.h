#pragma once

#ifndef SETTINGS_STATIC_LIB

#ifndef HUDSON_BUILD
    #define FILEVER          1,0,0,0
    #define PRODUCTVER       FILEVER
    #define STRFILEVER       "1,0,0,0"
    #define STRPRODUCTVER    STRFILEVER

    #define COMPANYNAME      "Pone" // UNDONE
    #define FILEDESCRIPTION  "Developer version of Settings library"
    #define INTERNALNAME     "Settings"
    #define LEGALCOPYRIGHT   "Copyright(c) 2010 - 2012"

    #ifdef DEBUG 
        #define ORIGINALFILENAME "SettingsX86d.dll"
    #else
        #define ORIGINALFILENAME "SettingsX86.dll"
    #endif

    #define PRODUCTNAME      "Settings library"
#else
    #define FILEVER          $$MAJOR$$,$$MINOR$$,$$HUDSON_BUILD$$,0
    #define PRODUCTVER       FILEVER
    #define STRFILEVER       "$$MAJOR$$,$$MINOR$$,$$HUDSON_BUILD$$,$$GIT_REVISION$$"
    #define STRPRODUCTVER    STRFILEVER

    #define COMPANYNAME      "$$COMPANYNAME$$"
    #define FILEDESCRIPTION  "$$FILEDESCRIPTION$$"
    #define INTERNALNAME     "$$INTERNALNAME$$"
    #define LEGALCOPYRIGHT   "$$LEGALCOPYRIGHT$$"
    #define ORIGINALFILENAME "$$FILENAME$$"
    #define PRODUCTNAME      "$$PRODUCTNAME$$"
#endif

#endif

