#include <iostream>
#include <srdevicepush.h>
#include <srreporter.h>
#include <sragent.h>
#include <srluapluginmanager.h>
#include <srlogger.h>
#include "integrate.h"
using namespace std;

static const char *srv = "simpleluaagent_1";
static const char *srt =
        "10,300,GET,/identity/externalIds/c8y_Serial/%%,,"
        "application/vnd.com.nsn.cumulocity.externalId+json,%%,STRING,\r\n"

        "10,301,POST,/inventory/managedObjects,"
        "application/vnd.com.nsn.cumulocity.managedObject+json,"
        "application/vnd.com.nsn.cumulocity.managedObject+json,%%,,\"{"
        "\"\"name\"\":\"\"SimpleLua-Agent\"\","
        "\"\"type\"\":\"\"c8y_simplelua\"\","
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


int main()
{
        const char *server = "http://developer.cumulocity.com";
        const char *deviceID = "387387";
        const char *path = "/tmp/c8ysimplelua";
        srLogSetLevel(SRLOG_DEBUG);
        Integrate igt;
        SrAgent agent(server, deviceID, &igt);
        if (agent.bootstrap(path))
                return 0;
        cerr << "Credential: " << agent.tenant() << '/'
             << agent.username() << ':' << agent.password() << endl;
        if (agent.integrate(srv, srt))
                return 0;
        cerr << "Device ID: " << agent.deviceID() << ", ID: " << agent.ID()
             << ", XID: " << agent.XID() << endl;
        SrLuaPluginManager lua(agent);
        lua.load("lua/my.lua");
        SrReporter reporter(server, agent.XID(), agent.auth(),
                            agent.egress, agent.ingress);
        SrDevicePush push(server, agent.XID(), agent.auth(),
                          agent.ID(), agent.ingress);
        if (reporter.start() || push.start())
                return 0;
        agent.loop();
        return 0;
}
