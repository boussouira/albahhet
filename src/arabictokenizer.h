#ifndef ARABICTOKENIZER_H
#define ARABICTOKENIZER_H

#include "cl_common.h"
#include "customCharTokenizer.h"

class ArabicTokenizer: public CustomCharTokenizer {
public:
        ArabicTokenizer(Reader* in);
        virtual ~ArabicTokenizer();

protected:
        bool isTokenChar(const TCHAR c) const;
        TCHAR normalize(const TCHAR chr) const;
};

#endif // ARABICTOKENIZER_H
