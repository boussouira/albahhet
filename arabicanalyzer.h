#ifndef ARABICANALYZER_H
#define ARABICANALYZER_H

#include "cl_common.h"

class ArabicAnalyzer: public Analyzer
{
public:
    ArabicAnalyzer();
    virtual ~ArabicAnalyzer();
    TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
    TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
};

#endif // ARABICANALYZER_H
