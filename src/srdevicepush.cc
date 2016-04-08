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
        bayeuxPolicy(1) {}


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
                switch (push->bayeuxPolicy) {
                case 0: ::sleep(2);
                        break;
                case 1: push->http.clear();
                        if (push->handshake() == -1) {
                                string err = "10000,";
                                if (push->http.response().empty()) {
                                        err += "0,";
                                        err += to_string(push->http.errNo);
                                } else {
                                        err += "1," + push->http.response();
                                }
                                push->queue.put(SrOpBatch(err));
                                ::sleep(10);
                                srWarning("push: handshake failed!");
                                break;
                        }
                case 2: push->http.clear();
                        if (push->subscribe() == -1) {
                                string err = "10001,";
                                if (push->http.response().empty()) {
                                        err += "0,";
                                        err += to_string(push->http.errNo);
                                } else {
                                        err = "1," + push->http.response();
                                }
                                push->queue.put(SrOpBatch(err));
                                ::sleep(10);
                                srWarning("push: subscribe failed!");
                                break;
                        }
                case 3: push->http.clear();
                        if (push->connect() == -1) {
                                string err = "10002,";
                                if (push->http.response().empty()) {
                                        err += "0,";
                                        err += to_string(push->http.errNo);
                                } else {
                                        err = "1," + push->http.response();
                                }
                                push->queue.put(SrOpBatch(err));
                                ::sleep(10);
                                srWarning("push: connect failed!");
                        } else {
                                SrOpBatch b(push->http.response());
                                push->process(b.data);
                                if (!push->isSleeping())
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
        size_t p1 = string::npos, p2 = string::npos, s1 = 0, s2 = 0;
        for (SrRecord r = sr.next(); r.size(); r = sr.next()) {
                if (r[0].second == "88") {
                        p1 = sr.pre;
                        bnum = strtoul(r[1].second.c_str(), NULL, 10);
                        s1 = sr.end - p1;
                } else if (r[0].second == "86") {
                        p2 = sr.pre;
                        bayeuxPolicy = r[4].second == "retry" ? 3 : 1;
                        s2 = sr.end - p2;
                }
        }
        if (p2 != string::npos)
                s.erase(p2, s2);
        if (p1 != string::npos)
                s.erase(p1, s1);
}
