#include "arabicfilter.h"

ArabicFilter::ArabicFilter(TokenStream* in, bool deleteTs)
    : TokenFilter(in,deleteTs)
{
}

ArabicFilter::~ArabicFilter()
{
}

Token* ArabicFilter::next(Token* token)
{
    if ( input->next(token) != NULL ){
        int32_t l = token->termLength();
        const TCHAR* chars = token->termBuffer();
        bool doProcess = false;
        for (int32_t i = 0; i < l; ++i) {
            if ( (0x064B <= chars[i] && chars[i] <=0x0653) || chars[i] == 0x0640 ) {
                doProcess = true;
                break;
            }
        }

        if ( !doProcess ) {
            return token;
        }

        StringBuffer output(l*2);
        for (int32_t j = 0; j < l; j++) {
            TCHAR c = chars[j];
            if(0x064B <= c && c <= 0x0653)
                continue;
            if(c == 0x0640) // TATWEEL
                continue;

            output.appendChar(c);
        }

        token->setText(output.getBuffer());
        return token;
    }

    return NULL;
}
