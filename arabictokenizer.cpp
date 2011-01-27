#include "arabictokenizer.h"

ArabicTokenizer::ArabicTokenizer(Reader* in) : CharTokenizer(in)
{
}
ArabicTokenizer::~ArabicTokenizer()
{
}

bool ArabicTokenizer::isTokenChar(const TCHAR c)  const
{
    if(_istspace(c))
        return false;
    else if(0x0621 <= c &&  c <= 0x06ED)
        return true;
    else
        return false;
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
