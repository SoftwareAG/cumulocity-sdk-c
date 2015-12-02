#include <srnethttp.h>
#include <srlogger.h>
#include <string>
using namespace std;

struct Progress {time_t start; time_t last;};


static int xferinfo(void *ptr, curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow)
{
        (void)dltotal;
        (void)ultotal;
        (void)ulnow;
        Progress *p = (Progress*)ptr;
        timespec tv;
        if (clock_gettime(CLOCK_MONOTONIC, &tv) == -1)
                return 0;
        if(tv.tv_sec - p->last >= 660) {
                p->last = tv.tv_sec;
                srDebug("HTTP recv counter: " + to_string(dlnow));
                if ((p->last - p->start) / 660 > dlnow) {
                        srError("Heartbeat miss!");
                        return -1;
                }
        }
        return 0;
}


/* for libcurl older than 7.32.0 */
static int progress(void *p, double dlt, double dln, double ult, double uln)
{
        return xferinfo(p, (curl_off_t)dlt, (curl_off_t)dln,
                        (curl_off_t)ult, (curl_off_t)uln);
}



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
        /* for libcurl older than 7.32.0 */
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
#ifdef CURLOPT_XFERINFOFUNCTION
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
#endif
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
}


SrNetHttp::~SrNetHttp() {curl_slist_free_all(chunk);}


int SrNetHttp::post(const std::string &request)
{
        timespec tv;
        clock_gettime(CLOCK_MONOTONIC, &tv);
        Progress p = {tv.tv_sec, tv.tv_sec};
        /* for libcurl older than 7.32.0 */
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &p);
#ifdef CURLOPT_XFERINFODATA
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &p);
#endif
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
