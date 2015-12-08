#include <fstream>
#include "srutils.h"
#include "smartrest.h"
using namespace std;


int readSrTemplate(const string &path, string &srv, string &srt)
{
        ifstream in(path);
        if (!(getline(in, srv)))
                return -1;
        for (string s2; getline(in, s2);) {
                if (s2.empty() || s2[0] == '#')
                        continue;
                srt += s2 + "\r\n";
        }
        return srt.empty();
}


int registerSrTemplate(const string &url, const string &auth,
                       string &srv, const string &srt)
{
        SrNetHttp http(url, srv, auth);
        return registerSrTemplate(http, srv, srt);
}


int registerSrTemplate(SrNetHttp &http, std::string &srv, const std::string &srt)
{
        if (http.post("") <= 0)
                return -1;

        SrLexer lex(http.response());
        SrLexer::SrToken t = lex.next();
        if (t.second == "40") {
                http.clear();
                if (http.post(srt) <= 0)
                        return -1;
                lex.reset(http.response());
                t = lex.next();
        }
        if (t.second == "20") {
                srv = lex.next().second;
                return 0;
        }
        return -1;
}
