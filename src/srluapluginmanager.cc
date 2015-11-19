#include "srluapluginmanager.h"
#include "srnetbinhttp.h"
#include "srlogger.h"
#define UNUSED(x) (void)x
using namespace std;


static int appendLuaPath(lua_State *L, const string &path)
{
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "path");
        string s = lua_tostring(L, -1);
        s += ";" + path;
        lua_pop(L, 1);
        lua_pushstring(L, s.c_str());
        lua_setfield(L, -2, "path");
        lua_pop(L, 1);
        return 0;
}


void SrLuaPluginManager::operator()(SrRecord &r, SrAgent &agent)
{
        UNUSED(agent);
        SrAgent::MsgID j = strtoul(r[0].second.c_str(), NULL, 10);
        _Handler::const_iterator it = handlers.find(j);
        if (it != handlers.end()) {
                const string &cb = it->second.second;
                getGlobal(it->second.first, cb.c_str())(r);
#ifdef DEBUG
        } else {
                srDebug("Lua: No handler for msg " + r[0].second);
#endif
        }
}


void SrLuaPluginManager::operator()(SrTimer &t, SrAgent &agent)
{
        UNUSED(agent);
        _Timer::const_iterator it = timers.find(&t);
        if (it != timers.end()) {
                const string &cb = it->second.second;
                getGlobal(it->second.first, cb.c_str())();
        }
}


int SrLuaPluginManager::load(const string &path)
{
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        appendLuaPath(L, packagePath);
        getGlobalNamespace(L)
                .beginClass<SrLuaPluginManager>("SrLuaPluginManager")
                .addFunction("addMsgHandler", &SrLuaPluginManager::addMsgHandler)
                .addFunction("addTimer", &SrLuaPluginManager::addTimer)
                .addFunction("send", &SrLuaPluginManager::send)
                .addFunction("post", &SrLuaPluginManager::post)
                .addFunction("postf", &SrLuaPluginManager::postf)
                .addFunction("get", &SrLuaPluginManager::get)
                .addFunction("getf", &SrLuaPluginManager::getf)
                .addProperty("server", &SrLuaPluginManager::server)
                .addProperty("ID", &SrLuaPluginManager::ID)
                .addProperty("resp", &SrLuaPluginManager::resp)
                .endClass()
                .beginClass<SrTimer>("SrTimer")
                .addFunction("start", &SrTimer::start)
                .addFunction("stop", &SrTimer::stop)
                .addProperty("interval", &SrTimer::interval, &SrTimer::setInterval)
                .endClass()
                .beginClass<SrRecord>("SrRecord")
                .addFunction("type", &SrRecord::typeInt)
                .addFunction("value", &SrRecord::value)
                .addProperty("size", &SrRecord::size)
                .endClass();
        push(L, this);
        lua_setglobal(L, "c8y");
        if (luaL_dofile(L, path.c_str()) == 0) {
                LuaRef f = getGlobal(L, "init");
                return f();
        }
        srError(lua_tostring(L, -1));
        return -1;
}


void SrLuaPluginManager::send(const string &s, LuaRef ref)
{
        int prio = ref.isNil() ? 0 : ref.cast<int>();
        agent.egress.put(SrNews(s, prio));
}


int SrLuaPluginManager::post(const string &dest, const string& ct,
                             const string &data)
{
        net.clear();
        return net.post(dest, ct, data);
}


int SrLuaPluginManager::postf(const string &dest, const string& ct,
                              const string &file)
{
        net.clear();
        return net.postf(dest, ct, file);
}


int SrLuaPluginManager::get(const string &id)
{
        net.clear();
        return net.get(id);
}


int SrLuaPluginManager::getf(const string &id, const string &dest)
{
        return net.getf(id, dest);
}


SrTimer *SrLuaPluginManager::addTimer(int interval, const string &callback,
                                      lua_State *L)
{
        SrTimer *t = new SrTimer(interval, this);
        timers[t] = make_pair(L, callback);
        agent.addTimer(*t);
        return t;
}


void SrLuaPluginManager::addMsgHandler(SrAgent::MsgID msgid,
                                       const string &callback, lua_State *L)
{
        handlers[msgid] = make_pair(L, callback);
        agent.addMsgHandler(msgid, this);
}
