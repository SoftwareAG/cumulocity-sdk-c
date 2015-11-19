#include <cstring>
#include <srnetsocket.h>
#include <srlogger.h>
#ifndef SOCK_RECV_BUF_SIZE
#define SOCK_RECV_BUF_SIZE 1024
#endif
using namespace std;


static int waitSocket(curl_socket_t sockfd, bool for_recv, int timeout)
{
        struct timeval tv = {timeout, 0};
        struct timeval *ptv = timeout ? &tv : NULL;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        fd_set *rdfds = for_recv ? &fds : NULL;
        fd_set *wrfds = for_recv ? NULL : &fds;
        return select(sockfd + 1, rdfds, wrfds, NULL, ptv);
}


SrNetSocket::SrNetSocket(const string &server): SrNetInterface(server)
{
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
}


int SrNetSocket::connect()
{
        curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
        errNo = curl_easy_perform(curl);
        if (errNo != CURLE_OK) {
                srWarning(string("SOCK connect: ") + _errMsg);
                return -1;
        }
#ifdef CURLINFO_ACTIVESOCKET
        errNo = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockextr);
#else  // Support for libcurl < 7.45.0
        errNo = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &sockextr);
#endif
        if (errNo != CURLE_OK) {
                srError(string("Sock connect: ") + _errMsg);
                return -1;
        }
        sockfd = sockextr;
        return errNo;
}


int SrNetSocket::send(const string &request)
{
        srDebug("SOCK send: " + request);
        if (waitSocket(sockfd, 0, timeout()) < 0) {
                srError(string("SOCK send: ") + strerror(errno));
                return -1;
        }
        const char *pch = request.c_str();
        const size_t s = request.size();
        errNo = CURLE_OK;
        for (size_t i = 0, n = 0; errNo == CURLE_OK && n < s;) {
                errNo = curl_easy_send(curl, pch + n, s - n, &i);
                n += i;
        }
        if (errNo == CURLE_OK) {
                srDebug("Sock send: OK!");
                return s;
        } else {
                srError(string("Sock send: (") + to_string(errNo) + ") "+ _errMsg);
                return -1;
        }
}


int SrNetSocket::recv(size_t len)
{
        const int c = waitSocket(sockfd, 1, timeout());
        if (c < 0) {
                srError(string("SOCK recv: ") + strerror(errno));
                return -1;
        }
        char buf[SOCK_RECV_BUF_SIZE];
        size_t n = 0;
        errNo = curl_easy_recv(curl, buf, len, &n);
        if (errNo == CURLE_OK) {
                resp.append(buf, n);
                srDebug("Sock recv: " + resp);
                return n;
        } else {
                srError(string("Sock recv: (") + to_string(errNo) + ") "+ _errMsg);
                return -1;
        }
}
