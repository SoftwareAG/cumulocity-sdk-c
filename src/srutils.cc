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


const string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz0123456789+/";


static size_t index(uint32_t x) {return (x & 0xfc0000) >> 18;}
static uint8_t _chr(uint32_t x) {return (x & 0xff0000) >> 16;}


static uint8_t base64(uint8_t c)
{
        if (isupper(c)) return c - 'A';
        else if (islower(c)) return c - 'a' + 26;
        else if (isdigit(c)) return c - '0' + 52;
        else if (c == '+') return 62;
        else if (c == '/') return 63;
        else return 0;
}


string b64Encode(const string &s)
{
        size_t i = 3;
        string ret;
        for (; i <= s.size(); i += 3) {
                uint8_t a = s[i - 3], b = s[i - 2], c = s[i - 1];
                uint32_t x = (a << 16) | (b << 8) | c;
                for (int j = 0; j < 4; ++j, x <<= 6)
                        ret += chars[index(x)];
        }
        const size_t k = i - s.size();
        if (k == 2) {
                uint8_t a = s[i - 3];
                uint32_t x = a << 16;
                for (int j = k; j < 4; ++j, x <<= 6)
                        ret += chars[index(x)];
                ret += "==";
        } else if (k == 1) {
                uint8_t a = s[i - 3], b = s[i - 2];
                uint32_t x = a << 16 | b << 8;
                for (int j = k; j < 4; ++j, x <<= 6)
                        ret += chars[index(x)];
                ret += '=';
        }
        return ret;
}


string b64Decode(const string &s)
{
        if (s.empty()) return "";
        string ret;
        size_t i = 4, m = s.size();
        while (s[m - 1] == '=') --m;
        for (; i <= m; i += 4) {
                uint8_t a = base64(s[i - 4]), b = base64(s[i - 3]);
                uint8_t c = base64(s[i - 2]), d = base64(s[i - 1]);
                uint32_t x = (a << 18) | (b << 12) | (c << 6) | d;
                for (int j = 0; j < 3; ++j, x <<= 8)
                        ret += _chr(x);
        }
        if (m + 1 == s.size()) {
                uint8_t a = base64(s[m-3]), b = base64(s[m-2]);
                uint32_t x = (a << 18) | (b << 12) | (base64(s[m - 1]) << 6);
                ret += _chr(x);
                ret += _chr(x << 8);
        } else if (m + 2 == s.size()) {
                uint8_t a = base64(s[m-2]), b = base64(s[m-1]);
                uint32_t x = (a << 18) | (b << 12);
                ret += _chr(x);
        }
        return ret;
}
