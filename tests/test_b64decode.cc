#include <iostream>
#include <cassert>
#include <srutils.h>
using namespace std;


int main()
{
        cerr << "Test base64 Decode: ";
        assert(b64Decode("TWFu") == "Man");
        assert(b64Decode("cGxlYXN1cmUu") == "pleasure.");
        assert(b64Decode("bGVhc3VyZS4=") == "leasure.");
        assert(b64Decode("ZWFzdXJlLg==") == "easure.");
        assert(b64Decode("YXN1cmUu") == "asure.");
        assert(b64Decode("c3VyZS4=") == "sure.");
        assert(b64Decode("Q3VlN1A2UFhTbg==") == "Cue7P6PXSn");
        cerr << "OK!" << endl;
        return 0;
}
