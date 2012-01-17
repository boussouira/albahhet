#include "arabicanalyzer.h"
#include "arabicfilter.h"
#include "arabictokenizer.h"
#include "wordtypefilter.h"

class SavedStreams : public TokenStream {
public:
    SavedStreams(): tokenStream(NULL), filteredTokenStream(NULL){}
    void close(){}
    Token* next(Token* /*token*/) {return NULL;}

    ArabicTokenizer* tokenStream;
    TokenStream* filteredTokenStream;
};

ArabicAnalyzer::ArabicAnalyzer()
{
}

ArabicAnalyzer::~ArabicAnalyzer()
{
    SavedStreams* streams = reinterpret_cast<SavedStreams*>(getPreviousTokenStream());
    if (streams != NULL) {
        if(streams->tokenStream)
            delete streams->tokenStream;

        if(streams->filteredTokenStream)
            delete streams->filteredTokenStream;
    }
}

TokenStream* ArabicAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader)
{
    TokenStream* ret;
    ret = _CLNEW ArabicTokenizer(reader);
    ret = _CLNEW ArabicFilter(ret, true);
    ret = _CLNEW WordTypeFilter(ret, true);

    return ret;
}

TokenStream* ArabicAnalyzer::reusableTokenStream(const TCHAR* /*fieldName*/, CL_NS(util)::Reader* reader)
{
    SavedStreams* streams = reinterpret_cast<SavedStreams*>(getPreviousTokenStream());
    if (streams == NULL) {
        streams = _CLNEW SavedStreams();
        setPreviousTokenStream(streams);

        streams->tokenStream = _CLNEW ArabicTokenizer(reader);
        streams->filteredTokenStream = _CLNEW ArabicFilter(streams->tokenStream, true);
        streams->filteredTokenStream = _CLNEW WordTypeFilter(streams->filteredTokenStream, true);
    } else {
        streams->tokenStream->reset(reader);
    }

    return streams->filteredTokenStream;
}
