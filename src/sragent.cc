#include <cstdlib>
#include <sragent.h>
#include <srlogger.h>
#include <curl/curl.h>
#include <signal.h>
#ifndef AGENT_VAL
#define AGENT_VAL 5
#endif
using namespace std;


static string b64Encode(const string &s)
{
        const unsigned m1 = 63 << 18, m2 = 63 << 12, m3 = 63 << 6;
        const string char_set = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz0123456789+/";
        string res;
        unsigned a = 0, l = static_cast<unsigned>(s.size());
        for (; l > 2; l -= 3) {
                unsigned d = s[a++] << 16;
                d |= s[a++] << 8;
                d |= s[a++];
                res.append(1, char_set.at((d & m1) >> 18));
                res.append(1, char_set.at((d & m2) >> 12));
                res.append(1, char_set.at((d & m3) >> 6));
                res.append(1, char_set.at(d & 63));
        }
        if(l == 2) {
                unsigned d = s[a++] << 16;
                d |= s[a++] << 8;
                res.append(1, char_set.at((d & m1) >> 18));
                res.append(1, char_set.at((d & m2) >> 12));
                res.append(1, char_set.at((d & m3) >> 6));
                res.append(1, '=');
        } else if(l == 1) {
                unsigned d = s[a++] << 16;
                res.append(1, char_set.at((d & m1) >> 18));
                res.append(1, char_set.at((d & m2) >> 12));
                res.append("==", 2);
        }
        return res;
}


static void ignoreSignal(int sig)
{
        struct sigaction sa;
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(sig, &sa, NULL);
}


SrAgent::SrAgent(const string &_server, const string &deviceid,
                 SrIntegrate *igt, SrBootstrap *boot):
        _server(_server), did(deviceid), pboot(boot), pigt(igt)
{
        curl_global_init(CURL_GLOBAL_DEFAULT);
        ignoreSignal(SIGPIPE);
}


SrAgent::~SrAgent() {curl_global_cleanup();}


int SrAgent::bootstrap(const string &path)
{
        int c = 0;
        if (pboot) {
                c = pboot->bootstrap(path);
                _tenant = pboot->tenant();
                _username = pboot->username();
                _password = pboot->password();
        } else {
                SrBootstrap boot(_server, did);
                c = boot.bootstrap(path);
                _tenant = boot.tenant();
                _username = boot.username();
                _password = boot.password();
        }
        _auth = "Authorization: Basic " + b64Encode(
                _tenant + "/" + _username + ":" + _password);
        return c;
}


int SrAgent::integrate(const string &srv, const string &srt)
{
        int c = pigt->integrate(*this, srv, srt);
        if (c == 0) {
                xid = pigt->XID();
                id = pigt->ID();
        }
        return c;
}


int SrAgent::send(const SrNews &news) {return egress.put(news);}


static string _com(const uint32_t xid, const string &id)
{
        return id + "@" + to_string(xid);
}


void SrAgent::processMessages()
{
        SrQueue<SrOpBatch>::Event e = ingress.get(200);
        if (e.second != SrQueue<SrOpBatch>::Q_OK) return;
        MsgXID m = strtoul(xid.c_str(), NULL, 10);
        MsgXID c = m;
        SmartRest sr(e.first.data);
        for (SrRecord r = sr.next(); r.size(); r = sr.next()) {
                MsgID j = strtoul(r[0].second.c_str(), NULL, 10);
                if (j == 87) {
                        c = strtoul(r[2].second.c_str(), NULL, 10);
                } else if (c == m) {
                        _Handler::iterator it = handlers.find(j);
                        if (it != handlers.end() && it->second) {
                                srDebug("Trigger Msg " + r[0].second);
                                (*it->second)(r, *this);
#ifdef DEBUG
                        } else {
                                srDebug("Drop Msg " + r[0].second);
#endif
                        }
                } else {
                        _XHandler::iterator it = sh.find(XMsgID(c, j));
                        if (it != sh.end() && it->second) {
                                srDebug("Trigger Msg " + _com(c, r[0].second));
                                (*it->second)(r, *this);
#ifdef DEBUG
                        } else {
                                srDebug("Drop Msg " + _com(c, r[0].second));
#endif
                        }
                }
        }
}


void SrAgent::loop()
{
        const timespec ts = {AGENT_VAL / 1000, (AGENT_VAL % 1000) * 1000000};
        while (true) {
                clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
                for (_TimerIter t = timers.begin(); t != timers.end(); ++t) {
                        timespec now;
                        clock_gettime(CLOCK_MONOTONIC, &now);
                        SrTimer *i = *t;
                        if (i->isActive() && i->fireTime() <= now) {
                                i->run(*this);
                                if (i->isActive())
                                        i->start();
                        }
                }
                processMessages();
        }
}
