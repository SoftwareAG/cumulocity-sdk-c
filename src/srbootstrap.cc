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

#include <fstream>
#include <unistd.h>

#include "srnethttp.h"
#include "smartrest.h"
#include "srbootstrap.h"

using namespace std;


#define MAX_CREDENTIAL_REQUESTS 255

int SrBootstrap::bootstrap(const std::string &path)
{
    if (loadCredential(path) == 0)
    {
        return 0;
    }
    else if (requestCredential() == 0)
    {
        return saveCredential(path);
    }

    return -1;
}


int SrBootstrap::loadCredential(const std::string &path)
{
    ifstream in(path);
    if (in >> _tenant >> _username >> _password)
    {
        return 0;
    }

    return -1;
}


int SrBootstrap::requestCredential()
{
    // user:management/devicebootstrap, password:Fhdt1bb1f'
    static const char *auth = "Authorization: Basic bWFuYWdlbWVudC9kZXZpY2Vib290c3RyYXA6RmhkdDFiYjFm";
    static const string post = "61," + _deviceID;
    SrNetHttp http(_server.c_str(), "", auth);
    int c = -1;

    for (unsigned short i = 0; i < MAX_CREDENTIAL_REQUESTS; ++i)
    {
        // wait for 2 seconds before next try
        sleep(2);

        // clear old content
        http.clear();

        // send POST request
        if (http.post(post) <= 0)
        {
            continue;
        }

        // parse response string
        SrLexer lexer(http.response());
        SrLexer::SrToken tok = lexer.next();

        if (tok.second == "70")
        {   // server response code 70 (device bootstrap polling response with credentials)

            tok = lexer.next();
            tok = lexer.next();
            tok = lexer.next();

            if (tok.second.empty())
            {
                break;
            }

            // copy tenant
            _tenant = tok.second;

            tok = lexer.next();
            if (tok.second.empty())
            {
                break;
            }

            // copy username
            _username = tok.second;

            tok = lexer.next();
            if (tok.second.empty())
            {
                break;
            }

            // copy password
            _password = tok.second;

            c = 0;
            break;
        }
    }

    return c;
}

int SrBootstrap::saveCredential(const std::string &path)
{
    ofstream out(path);
    string s = _tenant + "\n" + _username + "\n" + _password;

    if (out << s)
    {
        return 0;
    }

    return -1;
}
