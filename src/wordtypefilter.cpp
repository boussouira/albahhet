#include "wordtypefilter.h"

WordTypeFilter::WordTypeFilter(TokenStream* in, bool deleteTs)
    : TokenFilter(in,deleteTs)
{
    m_handleSaved = false;
}

WordTypeFilter::~WordTypeFilter()
{
}

Token* WordTypeFilter::next(Token* token)
{
    if(!m_savedBuffer.isEmpty()) {
        StringBuffer *output = m_savedBuffer.dequeue();
        token->setText(output->getBuffer());
        token->setPositionIncrement(0);
        delete output;

        return token;
    }

    if(input->next(token) != NULL) {
        int32_t termLen = token->termLength();
        const TCHAR* chars = token->termBuffer();
        bool doProcess = false;

        CharType prevChartype = ArabicChar;

        for(int32_t i = 0; i < termLen; ++i) {
            CharType chartype = getCharType(chars[i]);

            if(i>0) {
                if(chartype != prevChartype) {
                    doProcess = true;
                    break;
                }
            }

            prevChartype = chartype;
        }

        if(!doProcess)
            return token;

        bool doneFromToken = false;
        StringBuffer *output = new StringBuffer(termLen*2);

        for(int32_t j = 0; j < termLen; j++) {
            TCHAR c = chars[j];
            CharType chartype = getCharType(c);

            if(j>0) {
                if(chartype != prevChartype) {
                    if(!doneFromToken) {
                        token->setText(output->getBuffer());
                        doneFromToken = true;

                        delete output;
                        output = new StringBuffer(termLen*2);
                    } else {
                        m_savedBuffer.enqueue(output);

                        output = new StringBuffer(termLen*2);
                    }

                }
            }

            prevChartype = chartype;
            output->appendChar(c);
        }

        if(doProcess) {
            if(output->length()>0) {
                m_savedBuffer.enqueue(output);
            }
        }

        return token;
    }

    return NULL;
}

WordTypeFilter::CharType WordTypeFilter::getCharType(const TCHAR c)
{
    if(IS_ARABIC_CHAR(c))
        return ArabicChar;
    else if(IS_NUMBER(c))
        return NumberChar;
    else
        return LatinChar;
}
