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
        struct timeval tv = {timeout, 0};
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


SrNetSocket::SrNetSocket(const string &s): SrNetInterface(s), _server(s)
{
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
}


int SrNetSocket::connect()
{
        curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
        srInfo("Sock connect: " + _server);
        errNo = curl_easy_perform(curl);
        if (errNo != CURLE_OK) {
                if (_errMsg[0] == 0)
                        strcpy(_errMsg, strerror(errno));
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
        if (errNo != CURLE_OK) {
                srError(string("Sock send: ") + _errMsg);
                return -1;
        }
        int c = waitSocket(sockfd, 0, timeout());
        if (c < 0) {
                srError(string("Sock send: ") + strerror(errno));
                return -1;
        }
        errNo = CURLE_OK;
        size_t n = 0;
        errNo = curl_easy_send(curl, buf, len, &n);
        if (errNo == CURLE_OK) {
                return n;
        } else if (errNo != CURLE_AGAIN) {
                srError(string("Sock send: ") + _errMsg);
        }
        return -1;
}


int SrNetSocket::send(const string &request)
{
        return sendBuf(request.c_str(), request.size());
}


int SrNetSocket::recv(size_t len)
{
        curl_socket_t sockfd;
        errNo = getSocket(curl, sockfd);
        if (errNo != CURLE_OK) {
                srError(string("Sock recv: ") + _errMsg);
                return -1;
        }
        const int c = waitSocket(sockfd, 1, timeout());
        if (c < 0) {
                srError(string("Sock recv: ") + strerror(errno));
                return -1;
        }
        char buf[SR_SOCK_RXBUF_SIZE];
        size_t n = 0;
        errNo = curl_easy_recv(curl, buf, len, &n);
        if (errNo == CURLE_OK) {
                resp.append(buf, n);
                return n;
        } else if (errNo != CURLE_AGAIN) {
                srError(string("Sock recv: ") + _errMsg);
        }
        return -1;
}
