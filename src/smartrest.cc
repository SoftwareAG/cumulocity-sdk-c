#include "smartrest.h"
using namespace std;


SrLexer::SrToken SrLexer::next()
{
        SrLexer::SrToken tok;
        if (end == s.size()) {
                tok.first = SR_EOB;
                pre = start = end;
                return tok;
        }
        if (delimit) {
                pre = end;
                for (; end < s.size(); ++end) {
                        if (s[end] == ',') {
                                ++end;
                                break;
                        } else if (s[end] == '\n') {
                                start = end;
                                tok.first = SR_NEWLINE;
                                tok.second = s[end++];
                                delimit = false;
                                return tok;
                        }
                }
        }
        pre = end;
        for (;end < s.size() && !isgraph(s[end]) && s[end] != '\n'; ++end);
        start = end;
        bool escape = false;
        size_t digits = 0, others = 0, dots = 0;
        if (s[end] == '"') {
                escape = true;
                ++end;
        } else if (s[end] == '+' || s[end] == '-') {
                tok.second += s[end++];
        }
        for (; isprint(s[end]) || escape; ++end) {
                if (isdigit(s[end])) {
                        ++digits;
                } else if (s[end] == '.') {
                        ++dots;
                } else if (s[end] == '"') {
                        if (!escape)
                                break;
                        ++end;
                        if (s[end] == '"') {
                                ++others;
                        } else {
                                escape = false;
                                break;
                        }
                } else if (s[end] == ',' || s[end] == '\n') {
                        if (escape) ++others;
                        else break;
                } else {
                        ++others;
                }
                tok.second += s[end];
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
