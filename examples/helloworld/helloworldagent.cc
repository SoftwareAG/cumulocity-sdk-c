#include <sragent.h>
#include "integrate.h"
using namespace std;


static const char *srv = "helloworld_1";
static const char *srt =
        "10,300,GET,/identity/externalIds/c8y_Serial/%%,,"
        "application/vnd.com.nsn.cumulocity.externalId+json,%%,STRING,\r\n"

        "10,301,POST,/inventory/managedObjects,"
        "application/vnd.com.nsn.cumulocity.managedObject+json,"
        "application/vnd.com.nsn.cumulocity.managedObject+json,%%,,\"{"
        "\"\"name\"\":\"\"HelloWorld-Agent\"\",\"\"type\"\":\"\"c8y_helloworld\"\","
        "\"\"c8y_IsDevice\"\":{},\"\"com_cumulocity_model_Agent\"\":{}}\"\r\n"

        "10,302,POST,/identity/globalIds/%%/externalIds,application/json,,%%,"
        "STRING STRING,\"{\"\"externalId\"\":\"\"%%\"\","
        "\"\"type\"\":\"\"c8y_Serial\"\"}\"\r\n"

        "11,800,\"$.managedObject\",,\"$.id\"\r\n"
        "11,801,,\"$.c8y_IsDevice\",\"$.id\"\r\n";


int main()
{
        const char *server = "http://developer.cumulocity.com";
        const char *deviceID = "13344568";
        const char *credentialPath = "/tmp/c8yhelloworld";
        Integrate igt;
        SrAgent agent(server, deviceID, &igt);
        if (agent.bootstrap(credentialPath))
                return 0;
        if (agent.integrate(srv, srt))
                return 0;
        cout << "Credential: " << agent.tenant() << '/'
             <<agent.username() << ':' << agent.password() << endl;
        agent.loop();
        return 0;
}
