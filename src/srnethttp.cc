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

#include <srnethttp.h>
#include <srlogger.h>

using namespace std;

static int xferinfo(void *ptr, curl_off_t dltotal, curl_off_t dlnow,
        curl_off_t ultotal, curl_off_t ulnow)
{
    (void) dltotal;
    (void) ultotal;
    (void) ulnow;
    timespec tv;
    if (clock_gettime(CLOCK_MONOTONIC_COARSE, &tv) == -1)
    {
        return 0;
    }

    pair<time_t, time_t> *p = (pair<time_t, time_t> *) ptr;
    if (tv.tv_sec - p->second >= 660)
    {
        p->second = tv.tv_sec;
        srDebug("HTTP recv counter: " + to_string(dlnow));
        if ((p->second - p->first) / 660 > dlnow)
        {
            return -1;
        }

    }

    return 0;
}

/* for libcurl older than 7.32.0 */
static int progress(void *p, double dlt, double dln, double ult, double uln)
{
    return xferinfo(p, (curl_off_t) dlt, (curl_off_t) dln, (curl_off_t) ult,
            (curl_off_t) uln);
}

static size_t writeFunc(void *ptr, size_t size, size_t nmemb, void *data)
{
    const size_t n = size * nmemb;
    if (n > 0)
    {
        ((std::string*) data)->append((const char *) ptr, n);
    }

    return n;
}

static curl_slist *_init(const string &xid, const string &auth)
{
    curl_slist *chunk = curl_slist_append(NULL, "Accept:");
    chunk = curl_slist_append(chunk, "Content-Type:");
    chunk = curl_slist_append(chunk, auth.c_str());

    if (!xid.empty())
    {
        std::string s = "X-Id: " + xid;
        chunk = curl_slist_append(chunk, s.c_str());
    }

    return chunk;
}

SrNetHttp::SrNetHttp(const std::string &server, const std::string &xid,
        const std::string &auth) :
        SrNetInterface(server), chunk(NULL)
{
    chunk = _init(xid, auth);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

    // Progress meter, for device push heartbeat. <start, last>
    /* for libcurl older than 7.32.0 */
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &meter);
#ifdef CURLOPT_XFERINFOFUNCTION
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &meter);
#endif
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
}

SrNetHttp::~SrNetHttp()
{
    curl_slist_free_all(chunk);
}

int SrNetHttp::post(const std::string &request)
{
    srDebug("HTTP post: " + request);
    timespec tv = { 0, 0 };

    clock_gettime(CLOCK_MONOTONIC_COARSE, &tv);
    meter.first = meter.second = tv.tv_sec;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.size());
    errNo = curl_easy_perform(curl);

    if (errNo == CURLE_OK)
    {
        srDebug("HTTP recv: " + resp);
        long status;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        statusCode = status;
        return resp.size();
    } else
    {
        srError(string("HTTP post: ") + _errMsg);

        return -1;
    }
}
