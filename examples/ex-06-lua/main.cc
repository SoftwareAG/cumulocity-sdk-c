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
    const char* const server = "http://developer.cumulocity.com";
    const char* const credentialPath = "/tmp/helloc8y";
    const char* const deviceID = "13344568"; // unique device identifier

    srLogSetLevel(SRLOG_DEBUG);        // set log level to debug

    Integrate igt;
    SrAgent agent(server, deviceID, &igt); // instantiate SrAgent
    if (agent.bootstrap(credentialPath))   // bootstrap to Cumulocity
    {
        return 0;
    }

    string srversion, srtemplate;
    if (readSrTemplate("srtemplate.txt", srversion, srtemplate) != 0)
    {
        return 0;
    }

    if (agent.integrate(srversion, srtemplate)) // integrate to Cumulocity
    {
        return 0;
    }

    SrLuaPluginManager lua(agent);
    lua.addLibPath("lua/?.lua");  // add given path to Lua package.path
    lua.load("lua/myplugin.lua"); // load Lua plugin

    SrReporter reporter(server, agent.XID(), agent.auth(), agent.egress,  agent.ingress);
    if (reporter.start() != 0)      // Start the reporter thread
    {
        return 0;
    }

    // inform Cumulocity about supported operations
    agent.send("104," + agent.ID() + ",\"\"\"c8y_Restart\"\"\"");
    SrDevicePush push(server, agent.XID(), agent.auth(), agent.ID(), agent.ingress);
    if (push.start() != 0)      // Start the device push thread
    {
        return 0;
    }

    agent.loop();

    return 0;
}
