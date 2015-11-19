#include <srnethttp.h>
#include <srlogger.h>


static size_t writeFunc(void *ptr, size_t size, size_t nmemb, void *data)
{
        const size_t n = size * nmemb;
        if (n > 0)
                ((std::string*)data)->append((const char *)ptr, n);
        return n;
}


SrNetHttp::SrNetHttp(const std::string &server, const std::string &xid,
                     const std::string &auth):
        SrNetInterface(server), chunk(NULL)
{
        chunk = curl_slist_append(chunk, "Accept:");
        chunk = curl_slist_append(chunk, "Content-Type:");
        chunk = curl_slist_append(chunk, auth.c_str());
        if (!xid.empty()) {
                std::string s = "X-Id: " + xid;
                chunk = curl_slist_append(chunk, s.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
}


SrNetHttp::~SrNetHttp() {curl_slist_free_all(chunk);}


int SrNetHttp::post(const std::string &request)
{
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.size());
        srDebug("HTTP post: " + request);
        errNo = curl_easy_perform(curl);
        if (errNo == CURLE_OK) {
                srDebug("HTTP recv: " + resp);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
                return resp.size();
        }
        srError(string("HTTP post: ") + _errMsg);
        return -1;
}
