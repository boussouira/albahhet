#ifndef SETTINGSCHECKER_H
#define SETTINGSCHECKER_H

#include "common.h"
#include "shamelaindexinfo.h"
#include <qsettings.h>

class SettingsChecker
{
public:
    SettingsChecker();
    void checkIndexes();
    bool checkIndex(IndexInfoBase *index);
    void update();
    void updateToXml();
    void indexingConfig();
};

#endif // SETTINGSCHECKER_H
