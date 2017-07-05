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

#include <string>
#include <srnetinterface.h>
#include "srlogger.h"

using namespace std;

SrNetInterface::SrNetInterface(const string &server) :
        errNo(0), curl(NULL), t(0)
{
    *_errMsg = 0;
    _errMsg[sizeof(_errMsg) - 1] = 0;

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, server.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, _errMsg);
#if SR_CURL_SIGNAL == 0
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
#ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
#ifdef SR_HTTP_1_0
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
#endif
#if SR_SSL_VERIFYCERT == 0
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif
#ifdef SR_SSL_CACERT
    curl_easy_setopt(curl, CURLOPT_CAINFO, SR_SSL_CACERT);
#endif
}

SrNetInterface::~SrNetInterface()
{
    curl_easy_cleanup(curl);
}

void SrNetInterface::setTimeout(long timeout)
{
    t = timeout;
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

    srDebug("Net: setTimeout to " + to_string(timeout));
}

void SrNetInterface::setDebug(long l)
{
    curl_easy_setopt(curl, CURLOPT_VERBOSE, l);
}
