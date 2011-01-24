#ifndef CL_COMMON_H
#define CL_COMMON_H
#include <CLucene.h>
#include <CLucene/StdHeader.h>

#include <CLucene/_clucene-config.h>
#include <CLucene/config/repl_tchar.h>
#include <CLucene/config/repl_wchar.h>
#include <CLucene/util/CLStreams.h>
#include <CLucene/util/Misc.h>
#include <CLucene/util/StringBuffer.h>
#include <CLucene/util/dirent.h>
#include <CLucene/search/IndexSearcher.h>

#include <CLucene/highlighter/QueryTermExtractor.h>
#include <CLucene/highlighter/QueryScorer.h>
#include <CLucene/highlighter/Highlighter.h>
#include <CLucene/highlighter/TokenGroup.h>
#include <CLucene/highlighter/SimpleHTMLFormatter.h>
#include <CLucene/highlighter/SimpleFragmenter.h>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::search;
using namespace lucene::search::highlight;

class SimpleCssFormatter : public Formatter {
public:
    int numHighlights;

    SimpleCssFormatter() {
        numHighlights = 0;
    }

    ~SimpleCssFormatter() {
    }

    TCHAR* highlightTerm(const TCHAR* originalText, const TokenGroup* group) {
        if (group->getTotalScore() <= 0) {
            return STRDUP_TtoT(originalText);
        }
        numHighlights++; //update stats used in assertions

        int len = _tcslen(originalText) + 40;
        TCHAR* ret = _CL_NEWARRAY(TCHAR, len + 1);
        _tcscpy(ret, _T("<b style=\"background-color:#FFFF63\">"));
        _tcscat(ret, originalText);
        _tcscat(ret, _T("</b>"));

        return ret;
    }
};

#endif // CL_COMMON_H
