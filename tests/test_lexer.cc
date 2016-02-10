#include <iostream>
#include <cassert>
#include <smartrest.h>
using namespace std;


int main()
{
        cerr << "Test SrLexer: ";
        const string s = "+59, \"ab cc \"\"\",-.9\n,";
        SrLexer lex(s);
        SrLexer::SrToken tok = lex.next();
        assert(tok.first == SrLexer::SR_INT && tok.second == "+59");
        tok = lex.next();
        assert(tok.first == SrLexer::SR_STRING && tok.second == "ab cc \"");
        assert(lex.pre == 4);
        assert(lex.start == 5);
        assert(lex.end == 15);
        tok = lex.next();
        assert(tok.first == SrLexer::SR_FLOAT && tok.second == "-.9");
        tok = lex.next();
        assert(tok.first == SrLexer::SR_NEWLINE && tok.second == "\n");
        tok = lex.next();
        assert(tok.first == SrLexer::SR_NONE && tok.second == "");
        tok = lex.next();
        assert(tok.first == SrLexer::SR_NONE && tok.second == "");
        tok = lex.next();
        assert(tok.first == SrLexer::SR_EOB && tok.second == "");
        cerr << "OK!" << endl;
        return 0;
}
