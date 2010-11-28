#include "arabicanalyzer.h"

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

ArabicAnalyzer::ArabicAnalyzer(){
}
ArabicAnalyzer::~ArabicAnalyzer(){
}

TokenStream* ArabicAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader)
{
    //return _CLNEW ArabicFilter( _CLNEW ArabicTokenizer(reader), true);

    TokenStream* ret;
    ret = _CLNEW ArabicTokenizer(reader);
    ret = _CLNEW ArabicFilter( ret, true );

    return ret;
}

TokenStream* ArabicAnalyzer::reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader)
{
        ArabicFilter* tokenizer = static_cast<ArabicFilter*>(getPreviousTokenStream());
        if (tokenizer == NULL) {
            tokenizer = _CLNEW ArabicFilter( _CLNEW ArabicTokenizer(reader), true );
            setPreviousTokenStream(tokenizer);
        } else
                tokenizer->tokenizer()->reset(reader);
        return tokenizer;
}

ArabicFilter::ArabicFilter(TokenStream* _input, bool deleteTs)
    : TokenFilter(_input,deleteTs)
{
    m_input = static_cast<ArabicTokenizer*>(_input);
}

ArabicFilter::~ArabicFilter()
{
}

Token* ArabicFilter::next(Token* token)
{
        if ( m_input->next(token) != NULL ){
                int32_t l = token->termLength();
                const TCHAR* chars = token->termBuffer();
                bool doProcess = false;
                for (int32_t i = 0; i < l; ++i) {
                        if ( (0x064B <= chars[i] && chars[i] <=0x0653) || chars[i] == 0x0640 ) {
//                        if ( chars[i] >= 0x0620 && chars[i] <= 0x06DF ) {
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
                    if(c == 0x0640) // TATWEEL
                        continue;

                    output.appendChar(c);
                }

                token->setText(output.getBuffer());
                return token;
        }
        return NULL;
}

void ArabicFilter::reset(CL_NS(util)::Reader* _input)
{
    m_input->reset(_input);
}

