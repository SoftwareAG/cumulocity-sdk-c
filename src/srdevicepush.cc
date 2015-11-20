#include <unistd.h>
#include <cstring>
#include "smartrest.h"
#include "srdevicepush.h"
using namespace std;


static const char *H1 = "POST /devicecontrol/notifications HTTP/1.1\r\nHost: ";
static const char *H2 = "\r\n";
static const char *H3 = "\r\nX-Id: ";
static const char *H4 = "\r\nContent-Length: ";
static const char *H5 = "\r\n\r\n";


static string packHeader(const string &s, const string &xid, const string &a)
{
        size_t i = s.find("://");
        i = i == string::npos ? 0 : i + 3;
        return H1+s.substr(i)+H2+a+H3+xid+H4;
}


static string pack(const string &header, const string &body)
{
        return header + to_string(body.size()+2) + H5 + body + "\r\n";
}


static int parseHttpHeader(string &s)
{
        if (s.size() < 12) return -1;
        else if (s[9] != '2') return -1;

        const char delimiter[] = "\r\n\r\n";
        size_t pos = s.find(delimiter, 12);
        if (pos == string::npos)
                return -1;
        s.erase(0, pos + sizeof(delimiter) - 1);
        return 0;
}


SrDevicePush::SrDevicePush(const string &server, const string &xid,
                           const string &auth, const string &chn,
                           SrQueue<SrOpBatch> &queue):
        sock(server), queue(queue), channel(chn),
        header(packHeader(server, xid, auth)), bnum(0),
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
        bool connected = false;
        while (true) {
                if (push->sleeping) {
                        ::sleep(2);
                        continue;
                } else if (!connected && push->sock.connect()) {
                        ::sleep(5);
                        continue;
                }
                connected = true;
                switch (push->bayeuxPolicy) {
                case 1: push->sock.clear();
                        if (push->handshake() == -1) {
                                connected = false;
                                srWarning("push: handshake failed!");
                                break;
                        }
                case 2: push->sock.clear();
                        if (push->subscribe() == -1) {
                                connected = false;
                                srWarning("push: subscribe failed!");
                                break;
                        }
                case 3: push->sock.clear();
                        if (push->connect() == -1) {
                                connected = false;
                                srWarning("push: connect failed!");
                        } else {
                                SrOpBatch b(push->sock.response());
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
        if (sock.connect())
                return -1;
        sock.setTimeout(30);
        if (sock.send(pack(header, "80,true")) <= 0)
                return -1;
        if (sock.recv(512) <= 0)
                return -1;
        string s = sock.response();
        if (parseHttpHeader(s) == -1) {
                ::sleep(5);
                return -1;
        }
        SmartRest sr(s);
        SrRecord r = sr.next();
        if (r.size() == 1)
                bayeuxID = r[0].second;
        bnum = 0;
        return r.size() == 1 ? 0 : -1;
}


int SrDevicePush::subscribe()
{
        sock.setTimeout(30);
        if (sock.send(pack(header, "81," + bayeuxID + ",/" + channel)) <= 0)
                return -1;
        if (sock.recv(512) <= 0)
                return -1;
        string s = sock.response();
        if (parseHttpHeader(s) == -1) {
                ::sleep(5);
                return -1;
        }
        SrLexer lex(s);
        SrToken tok = lex.next();
        return lex.isdelimiter(tok) || tok.first == SR_NONE ? 0 : -1;
}


int SrDevicePush::connect()
{
        sock.setTimeout(30);
        string bstring = "," + to_string(bnum);
        if (bnum == 0) {
                bstring.clear();
                bayeuxPolicy = 3;
        }
        if (sock.send(pack(header, "83," + bayeuxID + bstring)) <= 0)
                return -1;
        sock.setTimeout(660);
        if (sock.recv(512) <= 0)
                return -1;
        const string &s = sock.response();
        if (s.size() < 12 || s[9] != '2') {
                ::sleep(5);
                return -1;
        } else if (s.compare(s.size() - 5, 5, "\r\n\r\n "))
                return 0;
        int c = 0;
        while ((c = sock.recv(512)) == 1);
        if (c < 1) return -1;
        else if (c < 512) return 0;
        sock.setTimeout(20);
        while ((c = sock.recv(512)) == 512);
        return 0;
}


void SrDevicePush::process(string &s)
{
        parseHttpHeader(s);
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
