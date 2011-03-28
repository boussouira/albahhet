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
    void update();
};

#endif // SETTINGSCHECKER_H
