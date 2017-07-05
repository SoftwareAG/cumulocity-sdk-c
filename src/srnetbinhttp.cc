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
#include <srnetbinhttp.h>
#include <srlogger.h>

#define SUFFIX "/inventory/binaries/"

using namespace std;

static const char* const objfmt = "{\"name\":\"%s\",\"type\":\"%s\"}";

static long getfilesize(const string &filename)
{
    ifstream in(filename, ios::ate | ios::binary);
    return in.tellg();
}

static void _formadd(struct curl_httppost **first, struct curl_httppost **last,
        const char *obj, const char *fz)
{
    curl_formadd(first, last, CURLFORM_COPYNAME, "object", CURLFORM_PTRCONTENTS, obj, CURLFORM_END);
    curl_formadd(first, last, CURLFORM_COPYNAME, "filesize", CURLFORM_PTRCONTENTS, fz, CURLFORM_END);
}

static size_t dumpFunc(void *data, size_t size, size_t nmemb, void *fptr)
{
    const size_t n = size * nmemb;

    if (n > 0 && !((ofstream*) fptr)->write((const char *) data, n))
    {
        return 0;
    }

    return n;
}

static size_t writeFunc(void *data, size_t size, size_t nmemb, void *sptr)
{
    const size_t n = size * nmemb;

    if (n > 0)
    {
        ((std::string*) sptr)->append((const char *) data, n);
    }

    return n;
}

SrNetBinHttp::SrNetBinHttp(const std::string &server, const std::string &auth) :
        SrNetInterface(server), server(server + SUFFIX), chunk(NULL)
{
    chunk = curl_slist_append(chunk, "Accept: application/json");
    chunk = curl_slist_append(chunk, auth.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
}

SrNetBinHttp::~SrNetBinHttp()
{
    curl_slist_free_all(chunk);
}

int SrNetBinHttp::post(const string &dest, const string &ct, const string &data)
{
    srInfo("BinHTTP post: name:" + dest + ", type:" + ct + ", size:" + to_string(data.size()));

    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    char obj[256] = { 0 };

    snprintf(obj, sizeof(obj), objfmt, dest.c_str(), ct.c_str());
    const string fz = to_string(data.size());
    _formadd(&formpost, &lastptr, obj, fz.c_str());
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file",
            CURLFORM_BUFFER, dest.c_str(), CURLFORM_BUFFERPTR, data.c_str(),
            CURLFORM_BUFFERLENGTH, data.size(), CURLFORM_CONTENTTYPE,
            ct.c_str(), CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_URL, server.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    errNo = curl_easy_perform(curl);
    curl_formfree(formpost);

    if (errNo == CURLE_OK)
    {
        srDebug("BinHTTP recv: " + resp);

        return resp.size();
    }

    srError(string("BinHTTP post: ") + _errMsg);

    return -1;
}

int SrNetBinHttp::postf(const string &dest, const string &ct, const string &file)
{
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    const string fz = to_string(getfilesize(file));

    srInfo("BinHTTP postf: name:" + dest + ", type:" + ct + ", size:" + fz.c_str() + " <- " + file);

    char obj[256] = { 0 };
    snprintf(obj, sizeof(obj), objfmt, dest.c_str(), ct.c_str());
    _formadd(&formpost, &lastptr, obj, fz.c_str());
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file",
            CURLFORM_FILENAME, dest.c_str(), CURLFORM_FILE, file.c_str(),
            CURLFORM_CONTENTTYPE, ct.c_str(), CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_URL, server.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    errNo = curl_easy_perform(curl);
    curl_formfree(formpost);

    if (errNo == CURLE_OK)
    {
        srDebug("BinHTTP recv: " + resp);
        return resp.size();
    }

    srError(string("BinHTTP postf: ") + _errMsg);

    return -1;
}

int SrNetBinHttp::get(const string &id)
{
    srInfo("BinHTTP get: " + id);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_URL, (server + id).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    errNo = curl_easy_perform(curl);

    if (errNo == CURLE_OK)
    {
        srInfo("BinHTTP get: response size:" + to_string(resp.size()));

        return resp.size();
    }

    srError(string("BinHTTP get: ") + _errMsg);

    return -1;
}

int SrNetBinHttp::getf(const string &id, const string &dest)
{
    srInfo("BinHTTP getf: " + id + " -> " + dest);

    ofstream out(dest);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dumpFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_URL, (server + id).c_str());
    errNo = curl_easy_perform(curl);

    if (errNo == CURLE_OK)
    {
        return out.tellp();
    }

    srError(string("BinHTTP getf: ") + _errMsg);

    return -1;
}
