
// ex-01-hello: src/main.cc
#include <iostream>
#include <sragent.h>
#include <srlogger.h>
using namespace std;

int main()
{
        const char *server = "http://developer.cumulocity.com";
        const char *credentialPath = "/tmp/helloc8y";
        const char *deviceID = "13344568";   // unique device identifier
        srLogSetLevel(SRLOG_DEBUG);          // set log level to debug
        SrAgent agent(server, deviceID);     // instantiate SrAgent
        if (agent.bootstrap(credentialPath)) // bootstrap to Cumulocity
                return 0;
        cout << "Hello, Cumulocity!" << endl;
        return 0;
}
