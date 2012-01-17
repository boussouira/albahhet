#ifndef WORDTYPEFILTER_H
#define WORDTYPEFILTER_H

#include "cl_common.h"
#include <QQueue>

class WordTypeFilter: public TokenFilter {
public:
    WordTypeFilter(TokenStream* in, bool deleteTs);
    virtual ~WordTypeFilter();

    enum CharType {
        ArabicChar = 1,
        LatinChar,
        NumberChar
    };

    Token* next(Token* token);
    CharType getCharType(const TCHAR c);

protected:
    Token *newTok(Token *orig, TCHAR *text);

protected:
    QQueue<StringBuffer*> m_savedBuffer;
    bool m_handleSaved;
};
#endif // WORDTYPEFILTER_H
