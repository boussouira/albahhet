#ifndef ARABICANALYZER_H
#define ARABICANALYZER_H

#include <CLucene.h>
#include <CLucene/StdHeader.h>
#include <CLucene/analysis/standard/StandardFilter.h>
#include <CLucene/_clucene-config.h>
#include <CLucene/config/repl_tchar.h>
#include <CLucene/config/repl_wchar.h>
#include <CLucene/util/CLStreams.h>
#include <CLucene/util/Misc.h>
#include <CLucene/config/repl_tchar.h>
#include <CLucene/util/StringBuffer.h>
#include <CLucene/util/dirent.h>
#include <CLucene/search/IndexSearcher.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cctype>
#include <string.h>
#include <algorithm>
#include <stdio.h>

using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::document;
using namespace lucene::search;

class ArabicAnalyzer: public Analyzer
{
public:
    ArabicAnalyzer();
    virtual ~ArabicAnalyzer();
    TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
    TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
};

class ArabicTokenizer: public CharTokenizer {
public:
        /** Construct a new ArabicTokenizer. */
        ArabicTokenizer(Reader* in);
        virtual ~ArabicTokenizer();
protected:
        /** Collects only characters which do not satisfy _istspace.*/
        bool isTokenChar(const TCHAR c) const;

        /** Collects only characters which satisfy _totlower. */
        TCHAR normalize(const TCHAR chr) const;
};

class ArabicFilter: public TokenFilter {
public:
        ArabicFilter(TokenStream* _input, bool deleteTs);
        virtual ~ArabicFilter();

        Token* next(Token* token);
        void reset(CL_NS(util)::Reader* _input);
        ArabicTokenizer* tokenizer() { return m_input; }
protected:
        ArabicTokenizer* m_input;
};

#endif // ARABICANALYZER_H
