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

#include <unistd.h>
#include <cstring>
#include "smartrest.h"
#include "srdevicepush.h"

using namespace std;

namespace
{
static const char* SERVER_PATH_NOTIFICATIONS = "/devicecontrol/notifications";
}

#define HANDSHAKE_TIMEOUT 30    // seconds
#define SUBSCRIBE_TIMEOUT 30    // seconds
#define CONNECT_TIMEOUT 0       // seconds

#define WAIT_TIME_AFTER_FAIL 10 // seconds


SrDevicePush::SrDevicePush(const string &server, const string &xid,
        const string &auth, const string &chn, SrQueue<SrOpBatch> &queue) :
        http(server + SERVER_PATH_NOTIFICATIONS, xid, auth), tid(0), bnum(0),
        queue(queue), channel(chn), bayeuxState(BAYEUX_STATE_HANDSHAKE)
{
}

int SrDevicePush::start()
{
    const int no = pthread_create(&tid, NULL, func, this);

    if (no)
    {
        srError(string("push: start failed, ") + strerror(no));

        return no;
    }

    srInfo("push: starting...");

    return no;
}

void *SrDevicePush::func(void *arg)
{
    SrDevicePush* const push = (SrDevicePush*) arg;

    while (true)
    {
        switch (push->bayeuxState)
        {
            case BAYEUX_STATE_IDLE:
            default:
            {
                ::sleep(2);
                break;
            }

            case BAYEUX_STATE_HANDSHAKE:
            {
                push->http.clear();

                if (push->handshake() == -1)
                {
                    string err = "10000,";
                    if (push->http.response().empty())
                    {
                        err += "0,";
                        err += to_string(push->http.errNo);
                    } else
                    {
                        err += "1," + push->http.response();
                    }

                    srWarning("push: handshake failed!");

                    push->queue.put(SrOpBatch(err));

                    // wait some time for retry
                    ::sleep(WAIT_TIME_AFTER_FAIL);

                    break;
                }
            }
            /* no break */

            case BAYEUX_STATE_SUBSCRIBE:
            {
                push->http.clear();

                if (push->subscribe() == -1)
                {
                    string err = "10001,";
                    if (push->http.response().empty())
                    {
                        err += "0,";
                        err += to_string(push->http.errNo);
                    } else
                    {
                        err = "1," + push->http.response();
                    }

                    srWarning("push: subscribe failed!");

                    push->queue.put(SrOpBatch(err));

                    // wait some time for retry
                    ::sleep(WAIT_TIME_AFTER_FAIL);

                    break;
                }
            }
            /* no break */

            case BAYEUX_STATE_CONNECT:
            {
                push->http.clear();

                if (push->connect() == -1)
                {
                    string err = "10002,";

                    if (push->http.response().empty())
                    {
                        err += "0,";
                        err += to_string(push->http.errNo);
                    } else
                    {
                        err = "1," + push->http.response();
                    }

                    srWarning("push: connect failed!");

                    push->queue.put(SrOpBatch(err));

                    // wait some time for retry
                    ::sleep(WAIT_TIME_AFTER_FAIL);
                } else
                {
                    SrOpBatch b(push->http.response());
                    push->process(b.data);

                    if (!push->isSleeping())
                    {
                        push->queue.put(b);
                    }
                }
            }
        }
    }

    return NULL;
}

int SrDevicePush::handshake()
{
    http.setTimeout(HANDSHAKE_TIMEOUT);

    if (http.post("80,true") <= 0)
    {
        return -1;
    }

    SmartRest sr(http.response());
    SrRecord r = sr.next();

    if (r.size() == 1)
    {
        bayeuxID = r[0].second;
    }

    bnum = 0;

    return r.size() == 1 ? 0 : -1;
}

int SrDevicePush::subscribe()
{
    http.setTimeout(SUBSCRIBE_TIMEOUT);

    const string request = "81," + bayeuxID + ",/" + channel + (xids.empty() ? "" : xids);

    if (http.post(request) < 0)
    {
        return -1;
    }

    SrLexer lex(http.response());
    SrLexer::SrToken tok = lex.next();

    return lex.isdelimiter(tok) || tok.first == SrLexer::SR_NONE ? 0 : -1;
}

int SrDevicePush::connect()
{
    http.setTimeout(CONNECT_TIMEOUT);

    string bstring = "," + to_string(bnum);

    if (bnum == 0)
    {
        bstring.clear();
        bayeuxState = BAYEUX_STATE_CONNECT;
    }

    if (http.post("83," + bayeuxID + bstring) < 0)
    {
        return -1;
    }

    return 0;
}

void SrDevicePush::process(string &s)
{
    SmartRest sr(s);
    size_t p1 = string::npos, p2 = string::npos, s1 = 0, s2 = 0;

    for (SrRecord r = sr.next(); r.size(); r = sr.next())
    {
        if (r[0].second == "88")
        {
            p1 = sr.pre;
            bnum = strtoul(r[1].second.c_str(), NULL, 10);
            s1 = sr.end - p1;
        } else if (r[0].second == "86")
        {   // Settings advice for the client using SmartREST real-time notifications.
            // timeout,interval,reconnect policy

            p2 = sr.pre;
            bayeuxState = r[4].second == "retry" ? BAYEUX_STATE_CONNECT : BAYEUX_STATE_HANDSHAKE; // none, retry, handshake
            s2 = sr.end - p2;
        }
    }

    if (p2 != string::npos)
    {
        s.erase(p2, s2);
    }

    if (p1 != string::npos)
    {
        s.erase(p1, s1);
    }
}
