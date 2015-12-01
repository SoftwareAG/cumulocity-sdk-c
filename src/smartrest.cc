#include "smartrest.h"
using namespace std;


SrLexer::SrToken SrLexer::next()
{
        SrLexer::SrToken tok;
        if (i == s.size()) {
                tok.first = SR_EOB;
                return tok;
        }
        if (delimit) {
                for (; i < s.size(); ++i) {
                        if (s[i] == ',') {
                                ++i;
                                break;
                        } else if (s[i] == '\n') {
                                tok.first = SR_NEWLINE;
                                tok.second = s[i++];
                                delimit = false;
                                return tok;
                        }
                }
        }
        for (;i < s.size() && !isgraph(s[i]) && s[i] != '\n'; ++i);
        bool escape = false;
        size_t digits = 0, others = 0, dots = 0;
        if (s[i] == '"') {
                escape = true;
                ++i;
        } else if (s[i] == '+' || s[i] == '-') {
                tok.second += s[i++];
        }
        for (; isprint(s[i]) || escape; ++i) {
                if (isdigit(s[i])) {
                        ++digits;
                } else if (s[i] == '.') {
                        ++dots;
                } else if (s[i] == '"') {
                        if (!escape)
                                break;
                        ++i;
                        if (s[i] == '"') {
                                ++others;
                        } else {
                                escape = false;
                                break;
                        }
                } else if (s[i] == ',' || s[i] == '\n') {
                        if (escape) ++others;
                        else break;
                } else {
                        ++others;
                }
                tok.second += s[i];
        }
        if (escape)
                tok.first = SR_ERROR;
        else if (others || dots > 1)
                tok.first = SR_STRING;
        else if (dots)
                tok.first = digits ? SR_FLOAT : SR_STRING;
        else if (digits)
                tok.first = SR_INT;
        else
                tok.first = tok.second.empty() ? SR_NONE : SR_STRING;
        delimit = true;
        return tok;
}
