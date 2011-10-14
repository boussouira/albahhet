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
#include <CLucene/search/QueryFilter.h>
#include <CLucene/queryParser/MultiFieldQueryParser.h>

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

#define MAX_SEGMENT 2

#define IS_UPPER(c) (0x41 <= c &&  c <= 0x5A)
#define IS_LOWER(c) (0x61 <= c &&  c <= 0x7A)
#define IS_LATIN(c) (0xC0 <= c && c <= 0xFF)
#define IS_ASCII(c) (IS_LOWER(c) || IS_UPPER(c) || IS_LATIN(c))
#define IS_NUMBER(c) (0x30 <= c &&  c <= 0x39)
#define IS_ARABIC_CHAR(c) (0x0621 <= c &&  c <= 0x06ED)

enum SearchSort {
    Relvance,
    BookRelvance,
    BookPage,
    DeathRelvance,
    DeathBookPage
};

#define PAGE_ID_FIELD _T("page")
#define BOOK_ID_FIELD _T("book")
#define PAGE_TEXT_FIELD _T("text")
#define FOOT_NOTE_FIELD _T("fn")
#define QURAN_SORA_FIELD _T("sora")
#define AUTHOR_DEATH_FIELD _T("death")

#endif // CL_COMMON_H
