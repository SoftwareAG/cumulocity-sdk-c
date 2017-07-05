/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "smartrest.h"

using namespace std;

SrLexer::SrToken SrLexer::next()
{
    SrLexer::SrToken tok;

    if (end == s.size())
    {
        tok.first = SR_EOB;
        pre = start = end;

        return tok;
    }

    if (delimit)
    {
        pre = end;

        for (; end < s.size(); ++end)
        {
            if (s[end] == ',')
            {
                ++end;
                break;
            } else if (s[end] == '\n')
            {
                start = end;

                tok.first = SR_NEWLINE;
                tok.second = s[end++];
                delimit = false;

                return tok;
            }
        }
    }

    pre = end;

    for (; end < s.size() && !isgraph(s[end]) && s[end] != '\n'; ++end)
    {
    }

    start = end;
    bool escape = false;
    size_t digits = 0, others = 0, dots = 0;

    if (s[end] == '"')
    {
        escape = true;
        ++end;
    } else if (s[end] == '+' || s[end] == '-')
    {
        tok.second += s[end++];
    }

    for (; isprint(s[end]) || escape; ++end)
    {
        if (isdigit(s[end]))
        {
            ++digits;
        } else if (s[end] == '.')
        {
            ++dots;
        } else if (s[end] == '"')
        {
            if (!escape)
            {
                break;
            }

            ++end;

            if (s[end] == '"')
            {
                ++others;
            } else
            {
                escape = false;
                break;
            }
        } else if (s[end] == ',' || s[end] == '\n')
        {
            if (escape)
            {
                ++others;
            }
            else
            {
                break;
            }
        } else
        {
            ++others;
        }

        tok.second += s[end];
    }

    if (escape)
    {
        tok.first = SR_ERROR;
    }
    else if (others || dots > 1)
    {
        tok.first = SR_STRING;
    }
    else if (dots)
    {
        tok.first = digits ? SR_FLOAT : SR_STRING;
    }
    else if (digits)
    {
        tok.first = SR_INT;
    }
    else
    {
        tok.first = tok.second.empty() ? SR_NONE : SR_STRING;
    }

    delimit = true;

    return tok;
}
