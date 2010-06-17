#include "arabicanalyzer.h"

ArabicTokenizer::ArabicTokenizer(Reader* in) : CharTokenizer(in)
{
}
ArabicTokenizer::~ArabicTokenizer()
{
}

bool ArabicTokenizer::isTokenChar(const TCHAR c)  const
{
//    bool ret;
    if(_istspace(c)) {  //(return true if NOT a space)
        return false;
    } else {
        if(0x21<=c && c<=0x40)
            return false;
        else if(0x5b<=c && c<=0x60)
            return false;
        else if(0x7b<=c && c<=0x7e)
            return false;
        else if(c==0x061B || c==0x061F || c==0x060C)
            return false;
        else
            return true;
/*
0x061B: ARABIC SEMICOLON
0x061F: ARABIC EXCLAMATION
0x060C: ARABIC COMMA
*/
    }
}

TCHAR ArabicTokenizer::normalize(const TCHAR chr) const
{
        return chr;
}

ArabicAnalyzer::ArabicAnalyzer(){
}
ArabicAnalyzer::~ArabicAnalyzer(){
}

TokenStream* ArabicAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader)
{
    //return _CLNEW ArabicFilter( _CLNEW ArabicTokenizer(reader), true);

    TokenStream* ret;

    ret = _CLNEW ArabicFilter( _CLNEW ArabicTokenizer(reader), true );

    ret = _CLNEW ::standard::StandardFilter(ret,true);
    ret = _CLNEW LowerCaseFilter(ret,true);
    return ret;

}
/*
TokenStream* ArabicAnalyzer::reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader)
{
        Tokenizer* tokenizer = static_cast<Tokenizer*>(getPreviousTokenStream());
        if (tokenizer == NULL) {
                tokenizer = _CLNEW ArabicTokenizer(reader);
                setPreviousTokenStream(tokenizer);
        } else
                tokenizer->reset(reader);
        return tokenizer;
}
*/
ArabicFilter::ArabicFilter(TokenStream* input, bool deleteTs):
        TokenFilter(input,deleteTs)
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
                        if ( chars[i] >= 0x0620 && chars[i] <= 0x06DF ) {
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
                    if(0x064B <= c && c <=0x0653)
                        continue;
                        switch (c) {
                        case 0x0640: // TATWEEL
                            break;
                        case 0x0622:
                        case 0x0623:
                        case 0x0625:
                            output.appendChar(0x0627);
                            break;
                        case 0x0629:
                            output.appendChar(0x0647);
                            break;
                        default :
                                output.appendChar(c);
                        break;
                    }
                }
                token->setText(output.getBuffer());
                return token;
        }
        return NULL;
}

