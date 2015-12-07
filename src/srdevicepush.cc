#include <unistd.h>
#include <cstring>
#include "smartrest.h"
#include "srdevicepush.h"
using namespace std;

const char *p = "/devicecontrol/notifications";

SrDevicePush::SrDevicePush(const string &server, const string &xid,
                           const string &auth, const string &chn,
                           SrQueue<SrOpBatch> &queue):
        http(server+p, xid, auth), bnum(0), queue(queue), channel(chn),
        bayeuxPolicy(1), sleeping(false) {}


int SrDevicePush::start()
{
        int no = pthread_create(&tid, NULL, func, this);
        if (no) {
                srError(string("push: start failed, ") + strerror(no));
                return no;
        }
        srInfo("push: started.");
        return no;
}


void *SrDevicePush::func(void *arg)
{
        SrDevicePush *push = (SrDevicePush*)arg;
        while (true) {
                if (push->sleeping) {
                        ::sleep(2);
                        continue;
                }
                switch (push->bayeuxPolicy) {
                case 1: push->http.clear();
                        if (push->handshake() == -1) {
                                ::sleep(10);
                                srWarning("push: handshake failed!");
                                break;
                        }
                case 2: push->http.clear();
                        if (push->subscribe() == -1) {
                                ::sleep(10);
                                srWarning("push: subscribe failed!");
                                break;
                        }
                case 3: push->http.clear();
                        if (push->connect() == -1) {
                                ::sleep(10);
                                srWarning("push: connect failed!");
                        } else {
                                SrOpBatch b(push->http.response());
                                push->process(b.data);
                                if (!push->sleeping)
                                        push->queue.put(b);
                        }
                }
        }
        return NULL;
}


int SrDevicePush::handshake()
{
        http.setTimeout(30);
        if (http.post("80,true") <= 0)
                return -1;
        SmartRest sr(http.response());
        SrRecord r = sr.next();
        if (r.size() == 1)
                bayeuxID = r[0].second;
        bnum = 0;
        return r.size() == 1 ? 0 : -1;
}


int SrDevicePush::subscribe()
{
        http.setTimeout(30);
        const string request = "81," + bayeuxID + ",/" + channel + (
                xids.empty() ? "" : xids);
        if (http.post(request) < 0)
                return -1;
        SrLexer lex(http.response());
        SrLexer::SrToken tok = lex.next();
        return lex.isdelimiter(tok) || tok.first == SrLexer::SR_NONE ? 0 : -1;
}


int SrDevicePush::connect()
{
        http.setTimeout(0);
        string bstring = "," + to_string(bnum);
        if (bnum == 0) {
                bstring.clear();
                bayeuxPolicy = 3;
        }
        if (http.post("83," + bayeuxID + bstring) < 0)
                return -1;
        return 0;
}


void SrDevicePush::process(string &s)
{
        SmartRest sr(s);
        size_t cur = 0;
        size_t p1 = string::npos, p2 = string::npos;
        for (SrRecord r = sr.next(); r.size(); r = sr.next()) {
                if (r[0].second == "86" && r.size() > 4) {
                        p1 = cur ? s.find("\n86,", cur - 1) : s.find("86,");
                        bayeuxPolicy =  r[4].second == "retry" ? 3 : 1;
                } else if (r[0].second == "88" && r.size() > 1) {
                        p2 = cur ? s.find("\n88,", cur - 1) : s.find("88,");
                        bnum = strtol(r[1].second.c_str(), NULL, 0);
                }
                for (size_t i = 0; i < r.size(); ++i)
                        cur += r[i].second.size() + 1;
        }
        const size_t min = p1 <= p2 ? p1 : p2;
        const size_t max = p1 <= p2 ? p2 : p1;
        if (max < s.size())
                s.erase(max, s.find('\n', max + 1) - max);
        if (min < s.size())
                s.erase(min, s.find('\n', min + 1) - min);
}
