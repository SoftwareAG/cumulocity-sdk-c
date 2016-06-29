
// ex-04-operation: src/main.cc
#include <cstdlib>
#include <sragent.h>
#include <srreporter.h>
#include <srlogger.h>
#include "srdevicepush.h"
#include "srutils.h"
#include "srluapluginmanager.h"
#include "integrate.h"
using namespace std;


int main()
{
        const char *server = "http://developer.cumulocity.com";
        const char *credentialPath = "/tmp/helloc8y";
        const char *deviceID = "13344568"; // unique device identifier
        srLogSetLevel(SRLOG_DEBUG);        // set log level to debug
        Integrate igt;
        SrAgent agent(server, deviceID, &igt); // instantiate SrAgent
        srDebug("bootstrap...");
        if (agent.bootstrap(credentialPath))   // bootstrap to Cumulocity
                return 0;
        string srversion, srtemplate;
        srDebug("read template...");
        if (readSrTemplate("srtemplate.txt", srversion, srtemplate) != 0)
                return 0;
        srDebug("integrate...");
        if (agent.integrate(srversion, srtemplate)) // integrate to Cumulocity
                return 0;

        SrLuaPluginManager lua(agent);
        lua.addLibPath("lua/?.lua");  // add given path to Lua package.path
        lua.load("lua/myplugin.lua"); // load Lua plugin

        SrReporter reporter(server, agent.XID(), agent.auth(),
                            agent.egress, agent.ingress);
        if (reporter.start() != 0)      // Start the reporter thread
                return 0;

        // Inform Cumulocity about supported operations
        agent.send("104," + agent.ID() + ",\"\"\"c8y_Restart\"\"\"");
        SrDevicePush push(server, agent.XID(), agent.auth(),
                          agent.ID(), agent.ingress);
        if (push.start() != 0)      // Start the device push thread
                return 0;

        agent.loop();
        return 0;
}
