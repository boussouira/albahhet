#include "customCharTokenizer.h"

CustomCharTokenizer::CustomCharTokenizer(Reader* in) :
    Tokenizer(in),
    offset(0),
    bufferIndex(0),
    dataLen(0),
    ioBuffer(NULL)
{
    buffer[0]=0;
}
CustomCharTokenizer::~CustomCharTokenizer(){
}

TCHAR CustomCharTokenizer::normalize(const TCHAR c) const
{
    return c;
}

Token* CustomCharTokenizer::next(Token* token){
    int32_t length = 0;
    int32_t start = offset;
    while (true) {
        TCHAR c;
        offset++;
        if (bufferIndex >= dataLen) {
            dataLen = input->read(ioBuffer, 1, LUCENE_IO_BUFFER_SIZE );
            if (dataLen == -1)
                dataLen = 0;
            bufferIndex = 0;
        }
        if (dataLen <= 0 ) {
            if (length > 0)
                break;
            else
                return NULL;
        } else
            c = ioBuffer[bufferIndex++];
        if (isTokenChar(c)) {                       // if it's a token TCHAR

            if (length == 0)			  // start of token
                start = offset-1;

            c = normalize(c);
            if(c)
                buffer[length++] = c;          // buffer it, normalized

            if (length == LUCENE_MAX_WORD_LEN)		  // buffer overflow!
                break;

        } else if (length > 0)			  // at non-Letter w/ chars
            break;					  // return 'em
    }

    buffer[length]=0;
    token->set(buffer, start, start+length);

    return token;
}
void CustomCharTokenizer::reset(CL_NS(util)::Reader* input)
{
    Tokenizer::reset(input);
    bufferIndex = 0;
    offset = 0;
    dataLen = 0;
}
