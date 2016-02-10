#include <iostream>
#include <cassert>
#include <smartrest.h>
using namespace std;


int main()
{
        cerr << "Test SmartREST: ";
        SrParser sr("a, b,\n+5.,hello world\n");
        SrRecord r = sr.next();
        assert(r.size() == 3);
        assert(r.value(0) == "a");
        assert(r.value(1) == "b");
        assert(r.value(2) == "");
        r = sr.next();
        assert(r.size() == 2);
        assert(r.value(0) == "+5." && r.type(0) == SrLexer::SR_FLOAT);
        assert(r.value(1) == "hello world");
        r = sr.next();
        assert(r.size() == 0);
        cerr << "OK!" << endl;
        return 0;
}
