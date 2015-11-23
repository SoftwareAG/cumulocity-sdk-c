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

/**
 *  \class SrLuaPluginManager srluapluginmanager.h
 *  \brief Lua plugin manager.
 *
 *  The SrLuaPluginManager manages all timer based and message ID based callbacks
 *  written in Lua. It acts as a bridge between the SrAgent and all Lua plugins,
 *  schedule these Lua callbacks on the agent's behalf. Additionally, the Lua
 *  manager also exposes many functionality, such as request sending, binary API,
 *  and many properties of the SrAgent, such as server URL, managed object ID,
 *  to all Lua plugins.
 *
 *  The Lua manager is access-able to all Lua plugins as an object named c8y.
 *  Beware in the following documentation, function annotated with "For Lua
 *  plugins only" are used by Lua plugins, should NOT be called in C++ program.
 */
class SrLuaPluginManager: public AbstractTimerFunctor, public AbstractMsgHandler
{
private:
        using string = std::string;
public:
        /**
         *  \brief SrLuaPluginManager constructor.
         *  \param agent reference to the SrAgent instance.
         */
        SrLuaPluginManager(SrAgent &agent):
                agent(agent), net(agent.server(), agent.auth()) {}
        virtual ~SrLuaPluginManager() {
                for (_TimerIter it = timers.begin(); it != timers.end(); ++it)
                        delete it->first;
        }

        /**
         *  \brief Bridging function for schedule all Lua message ID callbacks.
         *  \param r reference to the SmartREST record triggers this callback.
         *  \param agent reference to the SrAgent instance.
         */
        void operator()(SrRecord &r, SrAgent &agent);
        /**
         *  \brief Bridging function for schedule all Lua timer callbacks.
         *  \param timer reference to the SrTimer fires the callback.
         *  \param agent reference to the SrAgent instance.
         */
        void operator()(SrTimer &timer, SrAgent &agent);
        /**
         *  \brief Add a searching path for Lua libraries.
         *
         *  This function must be call before load a Lua plugin if you have to
         *  require Lua files not in the default searching path list.
         *
         *  \param path a lua package.path format path.
         */
        void addLibPath(const string &path) {packagePath += path + ";";}
        /**
         *  \brief Cumulocity server URL. For Lua plugins only.
         */
        const string &server() const {return agent.server();}
        /**
         *  \brief Agent managed object ID. For Lua plugins only.
         */
        const string &ID() const {return agent.ID();}
        /**
         *  \brief Get the response of the last performed HTTP binary API. For
         *  Lua plugins only.
         *
         *  Note you should first check the binary API's return code. The returned
         *  response is undefined if the last HTTP binary API failed.
         */
        const string &resp() const {return net.response();}
        /**
         *  \brief Load a Lua plugin.
         *  \param path path to the Lua plugin.
         *  \return 0 on success, non-0 otherwise.
         */
        int load(const string &path);
        /**
         *  \brief Wrapper of the SrAgent send function. For Lua plugins only.
         */
        void send(const string &s, LuaRef ref);
        /**
         *  \brief Wrapper of SrNetBinHTTP post. For lua plugins only.
         */
        int post(const string &dest, const string& ct, const string &data);
        /**
         *  \brief Wrapper of SrNetBinHTTP postf. For lua plugins only.
         */
        int postf(const string &dest, const string& ct, const string &file);
        /**
         *  \brief Wrapper of SrNetBinHTTP get. For lua plugins only.
         */
        int get(const string &id);
        /**
         *  \brief Wrapper of SrNetBinHTTP getf. For lua plugins only.
         */
        int getf(const string &id, const string &dest);
        /**
         *  \brief Add a timer to the agent. For Lua plugins only.
         *  \param interval period of the timer in milliseconds.
         *  \param callback name of callback function in the Lua plugin.
         *  \param L pointer to the calling Lua plugin.
         *  \return pointer to the created SrTimer, NULL on failure.
         */
        SrTimer *addTimer(int interval, const string &callback, lua_State *L);
        /**
         *  \brief Add a message ID based callback. For Lua plugins only.
         *  \param msgid message ID the callback registers to.
         *  \param callback name of callback function in the Lua plugin.
         *  \param L pointer to the calling Lua plugin.
         */
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
