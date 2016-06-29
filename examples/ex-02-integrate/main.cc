
// ex-02-integrate: src/main.cc
#include <sragent.h>
#include <srlogger.h>
#include "integrate.h"
using namespace std;

static const char *srversion = "helloc8y_1"; // SmartREST template version
static const char *srtemplate =              // SmartREST template collection
        "10,100,GET,/identity/externalIds/c8y_Serial/%%,,"
        "application/json,%%,STRING,\n"

        "10,101,POST,/inventory/managedObjects,application/json,"
        "application/json,%%,,\"{\"\"name\"\":\"\"HelloC8Y-Agent\"\","
        "\"\"type\"\":\"\"c8y_hello\"\",\"\"c8y_IsDevice\"\":{},"
        "\"\"com_cumulocity_model_Agent\"\":{}}\"\n"

        "10,102,POST,/identity/globalIds/%%/externalIds,application/json,,%%,"
        "STRING STRING,\"{\"\"externalId\"\":\"\"%%\"\","
        "\"\"type\"\":\"\"c8y_Serial\"\"}\"\n"

        "11,500,$.managedObject,,$.id\n"
        "11,501,,$.c8y_IsDevice,$.id\n";

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
        if (agent.integrate(srversion, srtemplate)) // integrate to Cumulocity
                return 0;
        agent.loop();
        return 0;
}
