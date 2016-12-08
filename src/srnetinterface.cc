#include <string>
#include <srnetinterface.h>
#include "srlogger.h"
using namespace std;


SrNetInterface::SrNetInterface(const string &server): errNo(0),curl(NULL), t(0)
{
        *_errMsg = 0;
        _errMsg[sizeof(_errMsg)-1] = 0;
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


SrNetInterface::~SrNetInterface() {curl_easy_cleanup(curl);}


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
