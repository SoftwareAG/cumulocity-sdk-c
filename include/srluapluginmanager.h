/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SRLUAPLUGINMANAGER_H
#define SRLUAPLUGINMANAGER_H

#include <algorithm>
#include <map>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "../ext/LuaBridge/Source/LuaBridge/LuaBridge.h"
#include "sragent.h"
#include "srnetbinhttp.h"

using namespace luabridge;

/**
 *  \class SrLuaPluginManager
 *  \brief Lua plugin manager.
 *
 *  The SrLuaPluginManager manages all timer based and message ID based callbacks
 *  written in Lua. It acts as a bridge between the SrAgent and all Lua plugins,
 *  schedule these Lua callbacks on the agent's behalf. Additionally, the Lua
 *  manager also exposes many functionality, such as request sending, binary API,
 *  and many properties of the SrAgent, such as server URL, managed object ID,
 *  to all Lua plugins. The Lua manager is access-able to all Lua plugins as
 *  an object named c8y.
 *  \note In the following documentation, function annotated with "For Lua
 *  plugins only" are intended only to be called in Lua plugins.
 */
class SrLuaPluginManager: public SrTimerHandler, public SrMsgHandler
{
private:
    using string = std::string;
public:
    /**
     *  \brief SrLuaPluginManager constructor.
     *  \param agent reference to the SrAgent instance.
     */
    SrLuaPluginManager(SrAgent &agent) :
            agent(agent), net(agent.server(), agent.auth())
    {
    }

    virtual ~SrLuaPluginManager();

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
    void addLibPath(const string &path)
    {
        packagePath += path + ";";
    }

    /**
     *  \brief Cumulocity server URL. For Lua plugins only.
     */
    const string &server() const
    {
        return agent.server();
    }

    /**
     *  \brief Agent managed object ID. For Lua plugins only.
     */
    const string &ID() const
    {
        return agent.ID();
    }
    /**
     *  \brief Get the response of the last performed HTTP binary API. For
     *  Lua plugins only.
     *  \note The return code of the performed binary API must be checked
     *  first before access this function.
     */

    const string &resp() const
    {
        return net.response();
    }

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
    void addMsgHandler(SrAgent::MsgID msgid, const string &callback, lua_State *L);

protected:
    /**
     *  \brief Initialize the Lua plugin.
     *
     *  This is a callback function, which is invoked by the load() function
     *  after successfully loads the Lua plugin and obtained the lua_State
     *  handle. This function is ought to be overridden when sub-classing
     *  SrLuaPluginManager, and tailor the C API exposure for Lua plugins.
     *
     *  \param L lua_State handle
     */
    virtual void init(lua_State *L);

private:

    typedef std::pair<lua_State*, std::string> _LuaCallback;
    typedef std::map<SrAgent::MsgID, _LuaCallback> _Handler;
    typedef std::map<SrTimer*, _LuaCallback> _Timer;
    _Handler handlers;
    _Timer timers;
    string packagePath;
    SrAgent &agent;
    SrNetBinHttp net;
};

#endif /* SRLUAPLUGINMANAGER_H */
