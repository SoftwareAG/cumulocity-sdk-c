#include <fstream>
#include "srutils.h"
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
