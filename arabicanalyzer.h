#ifndef ARABICANALYZER_H
#define ARABICANALYZER_H

#include "cl_common.h"

class ArabicAnalyzer: public Analyzer
{
public:
    ArabicAnalyzer();
    ArabicAnalyzer(const TCHAR **stopWord);
    ArabicAnalyzer(const char* stopwordsFile, const char* enc=NULL);
    virtual ~ArabicAnalyzer();

    TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
    TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);

private:
    CLTCSetList* stopSet;
};

#endif // ARABICANALYZER_H
