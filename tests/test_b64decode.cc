/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
