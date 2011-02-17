#ifndef SETTINGSCHECKER_H
#define SETTINGSCHECKER_H

#include "common.h"
#include <qsettings.h>

class SettingsChecker
{
public:
    SettingsChecker();
    void checkIndexes();
    bool checkIndex(QString index);
};

#endif // SETTINGSCHECKER_H
