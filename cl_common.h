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
        _tcscpy(ret, _T("<b style=\"background-color:#ffff63\">"));
        _tcscat(ret, originalText);
        _tcscat(ret, _T("</b>"));

        return ret;
    }
};

#define IS_NUMBER(c) (0x30 <= c &&  c <= 0x39)
#define IS_ARABIC_CHAR(c) (0x0621 <= c &&  c <= 0x06ED)

static const TCHAR *ARABIC_STOP_WORDS[] = {_T("من"),
                                           _T("ومن"),
                                           _T("منها"),
                                           _T("منه"),
                                           _T("في"),
                                           _T("وفي"),
                                           _T("فيها"),
                                           _T("فيه"),
                                           _T("و"),
                                           _T("ف"),
                                           _T("ثم"),
                                           _T("او"),
                                           _T("أو"),
                                           _T("ب"),
                                           _T("بها"),
                                           _T("به"),
                                           _T("ا"),
                                           _T("أ"),
                                           _T("اى"),
                                           _T("اي"),
                                           _T("أي"),
                                           _T("أى"),
                                           _T("لا"),
                                           _T("ولا"),
                                           _T("الا"),
                                           _T("ألا"),
                                           _T("إلا"),
                                           _T("لكن"),
                                           _T("ما"),
                                           _T("وما"),
                                           _T("كما"),
                                           _T("فما"),
                                           _T("عن"),
                                           _T("مع"),
                                           _T("اذا"),
                                           _T("إذا"),
                                           _T("ان"),
                                           _T("أن"),
                                           _T("إن"),
                                           _T("انها"),
                                           _T("أنها"),
                                           _T("إنها"),
                                           _T("انه"),
                                           _T("أنه"),
                                           _T("إنه"),
                                           _T("بان"),
                                           _T("بأن"),
                                           _T("فان"),
                                           _T("فأن"),
                                           _T("وان"),
                                           _T("وأن"),
                                           _T("وإن"),
                                           _T("التى"),
                                           _T("التي"),
                                           _T("الذى"),
                                           _T("الذي"),
                                           _T("الذين"),
                                           _T("الى"),
                                           _T("الي"),
                                           _T("إلى"),
                                           _T("إلي"),
                                           _T("على"),
                                           _T("عليها"),
                                           _T("عليه"),
                                           _T("اما"),
                                           _T("أما"),
                                           _T("إما"),
                                           _T("ايضا"),
                                           _T("أيضا"),
                                           _T("كل"),
                                           _T("وكل"),
                                           _T("لم"),
                                           _T("ولم"),
                                           _T("لن"),
                                           _T("ولن"),
                                           _T("هى"),
                                           _T("هي"),
                                           _T("هو"),
                                           _T("وهى"),
                                           _T("وهي"),
                                           _T("وهو"),
                                           _T("فهى"),
                                           _T("فهي"),
                                           _T("فهو"),
                                           _T("انت"),
                                           _T("أنت"),
                                           _T("لك"),
                                           _T("لها"),
                                           _T("له"),
                                           _T("هذه"),
                                           _T("هذا"),
                                           _T("تلك"),
                                           _T("ذلك"),
                                           _T("هناك"),
                                           _T("كانت"),
                                           _T("كان"),
                                           _T("يكون"),
                                           _T("تكون"),
                                           _T("وكانت"),
                                           _T("وكان"),
                                           _T("غير"),
                                           _T("بعض"),
                                           _T("قد"),
                                           _T("نحو"),
                                           _T("بين"),
                                           _T("بينما"),
                                           _T("منذ"),
                                           _T("ضمن"),
                                           _T("حيث"),
                                           _T("الان"),
                                           _T("الآن"),
                                           _T("خلال"),
                                           _T("بعد"),
                                           _T("قبل"),
                                           _T("حتى"),
                                           _T("عند"),
                                           _T("عندما"),
                                           _T("لدى"),
                                           _T("جميع"),
                                           NULL};
#endif // CL_COMMON_H
