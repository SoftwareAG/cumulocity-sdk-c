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

#include <iostream>
#include <cstdlib>
#include <sragent.h>
#include <srreporter.h>
#include <srlogger.h>
#include "srdevicepush.h"
#include "integrate.h"

using namespace std;

static const char *srversion = "helloc8y_3";
static const char *srtemplate =
        "10,100,GET,/identity/externalIds/c8y_Serial/%%,,"
        "application/json,%%,STRING,\n"

        "10,101,POST,/inventory/managedObjects,application/json,"
        "application/json,%%,,\"{\"\"name\"\":\"\"HelloC8Y-Agent\"\","
        "\"\"type\"\":\"\"c8y_hello\"\",\"\"c8y_IsDevice\"\":{},"
        "\"\"com_cumulocity_model_Agent\"\":{}}\"\n"

        "10,102,POST,/identity/globalIds/%%/externalIds,application/json,,%%,"
        "STRING STRING,\"{\"\"externalId\"\":\"\"%%\"\","
        "\"\"type\"\":\"\"c8y_Serial\"\"}\"\n"

        "10,103,POST,/measurement/measurements,application/json,,%%,"
        "NOW UNSIGNED NUMBER,\"{\"\"time\"\":\"\"%%\"\","
        "\"\"source\"\":{\"\"id\"\":\"\"%%\"\"},"
        "\"\"type\"\":\"\"c8y_CPUMeasurement\"\","
        "\"\"c8y_CPUMeasurement\"\":{\"\"Workload\"\":"
        "{\"\"value\"\":%%,\"\"unit\"\":\"\"%\"\"}}}\"\n"

        "10,104,PUT,/inventory/managedObjects/%%,application/json,,%%,"
        "UNSIGNED STRING,\"{\"\"c8y_SupportedOperations\"\":[%%]}\"\n"

        "10,105,PUT,/devicecontrol/operations/%%,application/json,,%%,"
        "UNSIGNED STRING,\"{\"\"status\"\":\"\"%%\"\"}\"\n"

        "11,500,$.managedObject,,$.id\n"
        "11,501,,$.c8y_IsDevice,$.id\n"
        "11,502,,$.c8y_Restart,$.id,$.deviceId\n";

class CPUMeasurement: public SrTimerHandler
{
public:

    CPUMeasurement()
    {
    }

    virtual ~CPUMeasurement()
    {
    }

    virtual void operator()(SrTimer &timer, SrAgent &agent)
    {
        const int cpu = rand() % 100;
        agent.send("103," + agent.ID() + "," + to_string(cpu));
    }
};

class RestartHandler: public SrMsgHandler
{
public:

    RestartHandler()
    {
    }

    virtual ~RestartHandler()
    {
    }

    virtual void operator()(SrRecord &r, SrAgent &agent)
    {
        agent.send("105," + r.value(2) + ",EXECUTING");
        for (int i = 0; i < r.size(); ++i)
        {
            cerr << r.value(i) << " ";
        }
        cerr << endl;

        agent.send("105," + r.value(2) + ",SUCCESSFUL");
    }
};

int main()
{
    const char* const server = "http://demos.cumulocity.com";
    const char* const credentialPath = "/tmp/helloc8y";
    const char* const deviceID = "13344568"; // unique device identifier

    srLogSetLevel(SRLOG_DEBUG);        // set log level to debug

    Integrate igt;
    SrAgent agent(server, deviceID, &igt); // instantiate SrAgent
    if (agent.bootstrap(credentialPath))   // bootstrap to Cumulocity
    {
        return 0;
    }

    if (agent.integrate(srversion, srtemplate)) // integrate to Cumulocity
    {
        return 0;
    }

    CPUMeasurement cpu;
    SrTimer timer(10 * 1000, &cpu); // Instantiate a SrTimer
    agent.addTimer(timer);          // Add the timer to agent scheduler
    timer.start();                  // Activate the timer

    SrReporter reporter(string(server) + ":1883", deviceID, agent.XID(),
            agent.tenant() + '/' + agent.username(), agent.password(),
            agent.egress, agent.ingress);

    // set MQTT keep-alive interval to 180 seconds.
    reporter.mqttSetOpt(SR_MQTTOPT_KEEPALIVE, 180);
    if (reporter.start() != 0)      // Start the reporter thread
    {
        return 0;
    }

    // Inform Cumulocity about supported operations
    agent.send("104," + agent.ID() + ",\"\"\"c8y_Restart\"\"\"");
    RestartHandler restartHandler;
    agent.addMsgHandler(502, &restartHandler);
    agent.loop();

    return 0;
}
