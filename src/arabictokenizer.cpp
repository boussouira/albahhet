#include "arabictokenizer.h"
#include "QDebug"

ArabicTokenizer::ArabicTokenizer(Reader* in) : CharTokenizer(in)
{
}
ArabicTokenizer::~ArabicTokenizer()
{
}

bool ArabicTokenizer::isTokenChar(const TCHAR c)  const
{
    return IS_ARABIC_CHAR(c) || IS_NUMBER(c) || IS_ASCII(c);
}

TCHAR ArabicTokenizer::normalize(const TCHAR chr) const
{
    TCHAR c = chr;

    switch(chr) {
    case 0x0622: // ALEF WITH MADDA ABOVE
    case 0x0623: // ALEF WITH HAMEZA ABOVE
    case 0x0625: // ALEF WITH HAMEZA BELOW
        c = 0x0627; // ALEF
        return c;

    case 0x0629: // TAH MARBUTA
        c = 0x0647; // HAH
        return c;

    case 0x0649: // ALEF MAKSURA
        c = 0x064A; // YEH
        return c;
    }

    if( chr >= 0xC0 && chr <= 0xFF ) {
        switch(chr) {
        case 0xC0 :
        case 0xC1 :
        case 0xC2 :
        case 0xC3 :
        case 0xC4 :
        case 0xC5 :
            c = 'a';
            break;
        case 0xC7 :
            c = 'c';
            break;
        case 0xC8 :
        case 0xC9 :
        case 0xCA :
        case 0xCB :
            c = 'e';
            break;
        case 0xCC :
        case 0xCD :
        case 0xCE :
        case 0xCF :
            c = 'i';
            break;
        case 0xD0 :
            c = 'd';
            break;
        case 0xD1 :
            c = 'n';
            break;
        case 0xD2 :
        case 0xD3 :
        case 0xD4 :
        case 0xD5 :
        case 0xD6 :
        case 0xD8 :
            c = 'o';
            break;
        case 0xD9 :
        case 0xDA :
        case 0xDB :
        case 0xDC :
            c = 'u';
            break;
        case 0xDD :
            c = 'Y';
            break;
        case 0xE0 :
        case 0xE1 :
        case 0xE2 :
        case 0xE3 :
        case 0xE4 :
        case 0xE5 :
            c = 'a';
            break;
        case 0xE7 :
            c = 'c';
            break;
        case 0xE8 :
        case 0xE9 :
        case 0xEA :
        case 0xEB :
            c = 'e';
            break;
        case 0xEC :
        case 0xED :
        case 0xEE :
        case 0xEF :
            c = 'i';
            break;
        case 0xF0 :
            c = 'd';
            break;
        case 0xF1 :
            c = 'n';
            break;
        case 0xF2 :
        case 0xF3 :
        case 0xF4 :
        case 0xF5 :
        case 0xF6 :
        case 0xF8 :
            c = 'o';
            break;
        case 0xF9 :
        case 0xFA :
        case 0xFB :
        case 0xFC :
            c = 'u';
            break;
        case 0xFD :
        case 0xFF :
            c = 'y';
            break;
        }
    }

    if(IS_UPPER(c)) {
        c = _totlower(c);
    }

    return c;
}
