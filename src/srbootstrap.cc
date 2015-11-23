#include <fstream>
#include <unistd.h>
#include "srnethttp.h"
#include "smartrest.h"
#include "srbootstrap.h"
using namespace std;


int SrBootstrap::bootstrap(const std::string &path)
{
        if (loadCredential(path) == 0)
                return 0;
        else if (requestCredential() == 0)
                return saveCredential(path);
        return -1;
}


int SrBootstrap::loadCredential(const std::string &path)
{
        ifstream in(path);
        if (in >> _tenant >> _username >> _password)
                return 0;
        return -1;
}


int SrBootstrap::requestCredential()
{
        const char *auth = "Authorization: Basic "
                "bWFuYWdlbWVudC9kZXZpY2Vib290c3RyYXA6RmhkdDFiYjFm";
        const string post = "61,"+ _deviceID;
        SrNetHttp http(_server.c_str(), "", auth);
        int c = -1;
        for (unsigned short i = 0; i < 255; ++i) {
                sleep(2);
                http.clear();
                if (http.post(post) <= 0)
                        continue;

                SrLexer lexer(http.response());
                SrLexer::SrToken tok = lexer.next();
                if (tok.second == "70") {
                        tok = lexer.next();
                        tok = lexer.next();
                        tok = lexer.next();
                        if (tok.second.empty())
                                break;
                        _tenant = tok.second;
                        tok = lexer.next();
                        if (tok.second.empty())
                                break;
                        _username = tok.second;
                        tok = lexer.next();
                        if (tok.second.empty())
                                break;
                        _password = tok.second;
                        c = 0;
                        break;
                }
        }
        return c;
}


int SrBootstrap::saveCredential(const std::string &path)
{
        ofstream out(path);
        string s = _tenant + "\n" + _username + "\n" + _password;
        if (out << s)
                return 0;
        return -1;
}
