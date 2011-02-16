#include "arabictokenizer.h"

ArabicTokenizer::ArabicTokenizer(Reader* in) : CharTokenizer(in)
{
}
ArabicTokenizer::~ArabicTokenizer()
{
}

bool ArabicTokenizer::isTokenChar(const TCHAR c)  const
{
    return (IS_ARABIC_CHAR(c) || IS_NUMBER(c));
}

TCHAR ArabicTokenizer::normalize(const TCHAR chr) const
{
    TCHAR c = chr;

    switch(chr){
    case 0x0622:
    case 0x0623:
    case 0x0625:
        c = 0x0627;
        break;

    case 0x0629:
        c = 0x0647;
        break;
    }

    return c;
}
