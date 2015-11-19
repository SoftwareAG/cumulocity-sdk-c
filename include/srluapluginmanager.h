#ifndef SRLUAPLUGINMANAGER_H
#define SRLUAPLUGINMANAGER_H
#include <map>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <LuaBridge.h>
#include "sragent.h"
#include "srnetbinhttp.h"
using namespace luabridge;


class SrLuaPluginManager: public AbstractTimerFunctor, public AbstractMsgHandler
{
public:
        using string = std::string;
        SrLuaPluginManager(SrAgent &agent):
                agent(agent), net(agent.server(), agent.auth()) {}
        virtual ~SrLuaPluginManager() {
                for (_TimerIter it = timers.begin(); it != timers.end(); ++it)
                        delete it->first;
        }

        void operator()(SrRecord &r, SrAgent &agent);
        void operator()(SrTimer &timer, SrAgent &agent);
        void addLibPath(const string &path) {packagePath += path + ";";}
        const string &server() const {return agent.server();}
        const string &ID() const {return agent.ID();}
        const string &resp() const {return net.response();}
        int load(const string &path);
        void send(const string &s, LuaRef ref);
        int post(const string &dest, const string& ct, const string &data);
        int postf(const string &dest, const string& ct, const string &file);
        int get(const string &id);
        int getf(const string &id, const string &dest);
        SrTimer *addTimer(int interval, const string &callback, lua_State *L);
        void addMsgHandler(SrAgent::MsgID msgid, const string &callback,
                           lua_State *L);

private:
        typedef std::pair<lua_State*, std::string> _LuaCallback;
        typedef std::map<SrAgent::MsgID, _LuaCallback> _Handler;
        typedef std::map<SrTimer*, _LuaCallback> _Timer;
        typedef _Timer::iterator _TimerIter;
        _Handler handlers;
        _Timer timers;
        string packagePath;
        SrAgent &agent;
        SrNetBinHttp net;
};

#endif /* SRLUAPLUGINMANAGER_H */
