#ifndef CUSTOMCHARTOKENIZER_H
#define CUSTOMCHARTOKENIZER_H

#include "cl_common.h"

class CustomCharTokenizer : public Tokenizer {
private:
        int32_t offset, bufferIndex, dataLen;
        TCHAR buffer[LUCENE_MAX_WORD_LEN+1];
        const TCHAR* ioBuffer;
protected:
        virtual bool isTokenChar(const TCHAR c) const = 0;
        virtual TCHAR normalize(const TCHAR c) const;

public:
        CustomCharTokenizer(CL_NS(util)::Reader* in);
        Token* next(Token* token);
        void reset(CL_NS(util)::Reader* input);

        virtual ~CustomCharTokenizer();
};

#endif // CUSTOMCHARTOKENIZER_H
