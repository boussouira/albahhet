#include "arabicanalyzer.h"
#include "arabicfilter.h"
#include "arabictokenizer.h"

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

ArabicAnalyzer::ArabicAnalyzer(const TCHAR **stopWord): stopSet(_CLNEW CLTCSetList(true))
{
    StopFilter::fillStopTable(stopSet, stopWord);
}

ArabicAnalyzer::ArabicAnalyzer(const char* stopwordsFile, const char* enc): stopSet(_CLNEW CLTCSetList(true))
{
    if(enc == NULL)
        enc = "UTF-8";

    WordlistLoader::getWordSet(stopwordsFile, enc, stopSet);
}

ArabicAnalyzer::~ArabicAnalyzer()
{
}

TokenStream* ArabicAnalyzer::tokenStream(const TCHAR* /*fieldName*/, Reader* reader)
{
    TokenStream* ret;
    ret = _CLNEW ArabicTokenizer(reader);
    ret = _CLNEW ArabicFilter( ret, true );
    ret = _CLNEW StopFilter(ret,true, stopSet, true);

    return ret;
}

TokenStream* ArabicAnalyzer::reusableTokenStream(const TCHAR* /*fieldName*/, CL_NS(util)::Reader* reader)
{
    SavedStreams* streams = reinterpret_cast<SavedStreams*>(getPreviousTokenStream());
    if (streams == NULL) {
        streams = _CLNEW SavedStreams();
        setPreviousTokenStream(streams);

        BufferedReader* bufferedReader = reader->__asBufferedReader();
        if ( bufferedReader == NULL )
            streams->tokenStream = _CLNEW ArabicTokenizer(_CLNEW FilteredBufferedReader(reader, false));
        else
            streams->tokenStream = _CLNEW ArabicTokenizer(bufferedReader);

        streams->filteredTokenStream = _CLNEW ArabicFilter(streams->tokenStream, true);
        streams->filteredTokenStream = _CLNEW StopFilter(streams->filteredTokenStream ,true, stopSet, true);
    } else {
        streams->tokenStream->reset(reader);
    }

    return streams->filteredTokenStream;
}
