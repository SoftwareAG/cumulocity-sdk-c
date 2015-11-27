#include <srnetinterface.h>


SrNetInterface::SrNetInterface(const string &server): errNo(0),curl(NULL), t(0)
{
        *_errMsg = 0;
        _errMsg[sizeof(_errMsg)-1] = 0;
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, server.c_str());
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, _errMsg);
#ifdef DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
}


SrNetInterface::~SrNetInterface() {curl_easy_cleanup(curl);}


void SrNetInterface::setTimeout(long timeout)
{
        t = timeout;
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
}


void SrNetInterface::setDebug(long l)
{
        curl_easy_setopt(curl, CURLOPT_VERBOSE, l);
}
