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
