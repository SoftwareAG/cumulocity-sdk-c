#include <cstdlib>
#include <iostream>
#include <srdevicepush.h>
#include <srreporter.h>
#include <sragent.h>
#include "integrate.h"
using namespace std;


static const char *srv = "simpleagent_1";
static const char *srt =
        "10,300,GET,/identity/externalIds/c8y_Serial/%%,,"
        "application/vnd.com.nsn.cumulocity.externalId+json,%%,STRING,\r\n"

        "10,301,POST,/inventory/managedObjects,"
        "application/vnd.com.nsn.cumulocity.managedObject+json,"
        "application/vnd.com.nsn.cumulocity.managedObject+json,%%,,\"{"
        "\"\"name\"\":\"\"Simple-Agent\"\",\"\"type\"\":\"\"c8y_simple\"\","
        "\"\"c8y_IsDevice\"\":{},\"\"com_cumulocity_model_Agent\"\":{},"
        "\"\"c8y_SupportedOperations\"\":[\"\"c8y_Relay\"\"]}\"\r\n"

        "10,302,POST,/identity/globalIds/%%/externalIds,application/json,,"
        "%%,STRING STRING,\"{\"\"externalId\"\":\"\"%%\"\","
        "\"\"type\"\":\"\"c8y_Serial\"\"}\"\r\n"

        "10,303,PUT,/devicecontrol/operations/%%,application/json,,%%,"
        "UNSIGNED STRING,\"{\"\"status\"\":\"\"%%\"\"}\"\r\n"

        "10,304,PUT,/devicecontrol/operations/%%,application/json,,%%,"
        "UNSIGNED STRING,\"{\"\"status\"\":\"\"FAILED\"\","
        "\"\"failureReason\"\":\"\"%%\"\"}\"\r\n"

        "10,305,POST,/measurement/measurements,application/json,,%%,"
        "NOW UNSIGNED NUMBER UNSIGNED,\"{\"\"time\"\":\"\"%%\"\","
        "\"\"source\"\":{\"\"id\"\":\"\"%%\"\"},"
        "\"\"type\"\":\"\"c8y_SignalStrength\"\","
        "\"\"c8y_SignalStrength\"\":{"
        "\"\"rssi\"\":{\"\"value\"\":\"\"%%\"\",\"\"unit\"\":\"\"dBm\"\"},"
        "\"\"ber\"\":{\"\"value\"\":\"\"%%\"\",\"\"unit\"\":\"\"%\"\"}}}\"\r\n"

        "11,800,\"$.managedObject\",,\"$.id\"\r\n"
        "11,801,,\"$.c8y_IsDevice\",\"$.id\"\r\n"

        "11,802,,\"$.deviceId\",\"$.id\",\"$.status\"\r\n"
        "11,803,,\"$.c8y_Relay\",\"$.id\",\"$.c8y_Relay.relayState\"\r\n";


class SignalTimer: public AbstractTimerFunctor
{
public:
        void operator()(SrTimer &timer, SrAgent &agent) {
                string s = "," + to_string((rand()%10) * (-5) - 40) + ",0";
                SrNews news;
                news.data = "305," + agent.ID() + s;
                agent.send(news);
        }
};


class RelayHandler: public AbstractMsgHandler
{
public:
        void operator()(SrRecord &r, SrAgent &agent) {
                SrNews news;
                if (r.size() == 4 && r[3].second == "CLOSED")
                        news.data = "303," + r[2].second + ",SUCCESSFUL";
                else if (r.size() == 4 && r[3].second == "OPEN")
                        news.data = "304," + r[2].second + ",Don't do it!";
                else
                        news.data = "304," + r[2].second + ",Wrong operation.";
                agent.send(news);
        }
};


int main()
{
        const char *server = "http://developer.cumulocity.com";
        const char *deviceID = "987654";
        const char *path = "/tmp/c8ysimple";
        Integrate igt;
        SrAgent agent(server, deviceID, &igt);
        if (agent.bootstrap(path))
                return 0;
        cout << agent.tenant() << '/' << agent.username()
             << ':' << agent.password() << endl;
        if (agent.integrate(srv, srt))
                return 0;
        cout << "Device ID: " << agent.deviceID() << ", ID: " << agent.ID()
             << ", XID: " << agent.XID() << endl;
        SrReporter reporter(server, agent.XID(), agent.auth(), agent.egress,
                            agent.ingress);
        SrDevicePush push(server, agent.XID(), agent.auth(), agent.ID(),
                          agent.ingress);
        if (reporter.start() || push.start())
                return 0;
        RelayHandler relayHandler;
        agent.addMsgHandler(803, &relayHandler);
        SignalTimer signal;
        SrTimer timer(120 * 1000, &signal);
        agent.addTimer(timer);
        timer.start();
        agent.loop();
        return 0;
}
