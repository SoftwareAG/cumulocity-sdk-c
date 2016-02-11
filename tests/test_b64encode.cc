#include <iostream>
#include <cassert>
#include <srutils.h>
using namespace std;


int main()
{
        cerr << "Test base64 Encode: ";
        assert(b64Encode("Man") == "TWFu");
        assert(b64Encode("pleasure.") == "cGxlYXN1cmUu");
        assert(b64Encode("leasure.") == "bGVhc3VyZS4=");
        assert(b64Encode("easure.") == "ZWFzdXJlLg==");
        assert(b64Encode("asure.") == "YXN1cmUu");
        assert(b64Encode("sure.") == "c3VyZS4=");
        assert(b64Encode("Cue7P6PXSn") == "Q3VlN1A2UFhTbg==");
        cerr << "OK!" << endl;
        return 0;
}
