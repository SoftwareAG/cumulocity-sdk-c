#ifndef SRAGENT_H
#define SRAGENT_H
#include <map>
#include "smartrest.h"
#include "srqueue.h"
#include "srbootstrap.h"
#include "srintegrate.h"
#include "srtimer.h"

int readSrTemplate(const std::string &path, std::string &srv, std::string &srt);


class AbstractMsgHandler
{
public:
        virtual ~AbstractMsgHandler() {}
        virtual void operator()(SrRecord &r, SrAgent &agent) = 0;
};


class SrAgent
{
public:
        typedef uint16_t MsgID;
        using string = std::string;
        SrAgent(const string &_server, const string &deviceid,
                SrIntegrate *igt = NULL, SrBootstrap *boot = NULL);
        virtual ~SrAgent();

        const string &tenant() const { return _tenant; }
        const string &username() const { return _username; }
        const string &password() const { return _password; }
        const string &auth() const { return _auth; }
        const string &server() const { return _server; }
        const string &deviceID() const { return did; }
        const string &XID() const { return xid; }
        const string &ID() const { return id; }
        int bootstrap(const string &path);
        int integrate(const string &srv, const string &srt);
        int send(const SrNews &news);
        void loop();
        void addTimer(SrTimer &timer) {timers.push_back(&timer);}
        void addMsgHandler(MsgID msgid, AbstractMsgHandler *functor) {
                handlers[msgid] = functor;
        }

public:
        SrQueue<SrOpBatch> ingress;
        SrQueue<SrNews> egress;

private:
        typedef AbstractMsgHandler* _Callback;
        typedef std::map<MsgID, _Callback> _Handler;
        typedef std::vector<SrTimer*> _Timer;
        typedef _Timer::iterator _TimerIter;
        _Timer timers;
        _Handler handlers;
        string _tenant;
        string _username;
        string _password;
        string _auth;
        const string _server;
        const string did;
        string xid;
        string id;
        SrBootstrap *pboot;
        SrIntegrate *pigt;
};

#endif /* SRAGENT_H */
