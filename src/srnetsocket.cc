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

#include <cstring>
#include <srnetsocket.h>
#include <srlogger.h>

using namespace std;

static int getSocket(CURL* curl, curl_socket_t &sockfd)
{
    long sockextr;
#ifdef CURLINFO_ACTIVESOCKET
    int c = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockextr);
#else  // Support for libcurl < 7.45.0
    int c = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &sockextr);
#endif
    sockfd = sockextr;
    return c;
}

static int waitSocket(curl_socket_t sockfd, bool for_recv, int timeout)
{
    struct timeval tv =
    { timeout, 0 };
    struct timeval *ptv = timeout ? &tv : NULL;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    fd_set *rdfds = for_recv ? &fds : NULL;
    fd_set *wrfds = for_recv ? NULL : &fds;

    // Do not test for select() == 0, some devices has quirky libcurl or
    // driver, select() returns 0 even there is data to read, rely on
    // curl_easy_send() and curl_easy_recv() with CURLE_AGAIN for testing.
    return select(sockfd + 1, rdfds, wrfds, NULL, ptv);
}

SrNetSocket::SrNetSocket(const string &s) :
        SrNetInterface(s), _server(s)
{
    // dead connections consume significant mem when using SSL
    curl_easy_setopt(curl, CURLOPT_MAXCONNECTS, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
}

int SrNetSocket::connect()
{
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
    srInfo("Sock connect: " + _server);
    errNo = curl_easy_perform(curl);

    if (errNo != CURLE_OK)
    {
        if (_errMsg[0] == 0)
        {
            strcpy(_errMsg, strerror(errno));
        }

        srError(string("Sock connect: ") + _errMsg);
        return -1;
    }
    srDebug("Sock connect: OK!");

    return errNo;
}

int SrNetSocket::sendBuf(const char *buf, size_t len)
{
    curl_socket_t sockfd;
    errNo = getSocket(curl, sockfd);

    if (errNo != CURLE_OK)
    {
        srError(string("Sock send: ") + _errMsg);
        return -1;
    }

    const int c = waitSocket(sockfd, 0, timeout());

    if (c < 0)
    {
        srError(string("Sock send: ") + strerror(errno));
        return -1;
    }

    errNo = CURLE_OK;
    size_t n = 0;
    errNo = curl_easy_send(curl, buf, len, &n);

    if (errNo == CURLE_OK || errNo == CURLE_AGAIN)
    {
        return n;
    } else
    {
        srError(string("Sock send: ") + _errMsg);
        return -1;
    }
}

int SrNetSocket::send(const string &request)
{
    return sendBuf(request.c_str(), request.size());
}

int SrNetSocket::recv(size_t len)
{
    curl_socket_t sockfd;
    errNo = getSocket(curl, sockfd);

    if (errNo != CURLE_OK)
    {
        srError(string("Sock recv: ") + _errMsg);
        return -1;
    }

    const int c = waitSocket(sockfd, 1, timeout());

    if (c < 0)
    {
        srError(string("Sock recv: ") + strerror(errno));
        return -1;
    }

    char buf[SR_SOCK_RXBUF_SIZE];
    size_t n = 0;
    errNo = curl_easy_recv(curl, buf, len, &n);

    if (errNo == CURLE_OK)
    {
        resp.append(buf, n);
        return n;
    } else if (errNo != CURLE_AGAIN)
    {
        srError(string("Sock recv: ") + _errMsg);
    }

    return -1;
}
