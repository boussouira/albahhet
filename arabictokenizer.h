#ifndef ARABICTOKENIZER_H
#define ARABICTOKENIZER_H

#include "cl_common.h"

class ArabicTokenizer: public CharTokenizer {
public:
        /** Construct a new ArabicTokenizer. */
        ArabicTokenizer(Reader* in);
        virtual ~ArabicTokenizer();
protected:
        /** Collects only characters which do not satisfy _istspace.*/
        bool isTokenChar(const TCHAR c) const;

        /** Collects only characters which satisfy _totlower. */
        TCHAR normalize(const TCHAR chr) const;
};

#endif // ARABICTOKENIZER_H
