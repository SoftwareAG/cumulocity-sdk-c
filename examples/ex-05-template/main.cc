
// ex-04-operation: src/main.cc
#include <iostream>
#include <cstdlib>
#include <sragent.h>
#include <srreporter.h>
#include <srlogger.h>
#include "srdevicepush.h"
#include "srutils.h"
#include "integrate.h"
using namespace std;

class CPUMeasurement: public SrTimerHandler {
public:
        CPUMeasurement() {}
        virtual ~CPUMeasurement() {}
        virtual void operator()(SrTimer &timer, SrAgent &agent) {
                const int cpu = rand() % 100;
                agent.send("103," + agent.ID() + "," + to_string(cpu));
        }
};

class RestartHandler: public SrMsgHandler {
public:
        RestartHandler() {}
        virtual ~RestartHandler() {}
        virtual void operator()(SrRecord &r, SrAgent &agent) {
                agent.send("105," + r.value(2) + ",EXECUTING");
                for (int i = 0; i < r.size(); ++i)
                        cerr << r.value(i) << " ";
                cerr << endl;
                agent.send("105," + r.value(2) + ",SUCCESSFUL");
        }
};

int main()
{
        const char *server = "http://developer.cumulocity.com";
        const char *credentialPath = "/tmp/helloc8y";
        const char *deviceID = "13344568"; // unique device identifier
        srLogSetLevel(SRLOG_DEBUG);        // set log level to debug
        Integrate igt;
        SrAgent agent(server, deviceID, &igt); // instantiate SrAgent
        if (agent.bootstrap(credentialPath))   // bootstrap to Cumulocity
                return 0;
        string srversion, srtemplate;
        if (readSrTemplate("srtemplate.txt", srversion, srtemplate) != 0)
                return 0;
        if (agent.integrate(srversion, srtemplate)) // integrate to Cumulocity
                return 0;

        CPUMeasurement cpu;
        SrTimer timer(10 * 1000, &cpu); // Instantiate a SrTimer
        agent.addTimer(timer);          // Add the timer to agent scheduler
        timer.start();                  // Activate the timer
        SrReporter reporter(server, agent.XID(), agent.auth(),
                            agent.egress, agent.ingress);
        if (reporter.start() != 0)      // Start the reporter thread
                return 0;

        // Inform Cumulocity about supported operations
        agent.send("104," + agent.ID() + ",\"\"\"c8y_Restart\"\"\"");
        RestartHandler restartHandler;
        agent.addMsgHandler(502, &restartHandler);
        SrDevicePush push(server, agent.XID(), agent.auth(),
                          agent.ID(), agent.ingress);
        if (push.start() != 0)      // Start the device push thread
                return 0;
        agent.loop();
        return 0;
}
