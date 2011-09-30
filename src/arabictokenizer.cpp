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
    case 0x0622: // ALEF WITH MADDA ABOVE
    case 0x0623: // ALEF WITH HAMEZA ABOVE
    case 0x0625: // ALEF WITH HAMEZA BELOW
        c = 0x0627; // ALEF
        break;

    case 0x0629: // TAH MARBUTA
        c = 0x0647; // HAH
        break;

   case 0x0649: // ALEF MAKSURA
       c = 0x064A; // YEH
       break;
    }

    return c;
}
