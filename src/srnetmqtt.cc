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

#include <sstream>
#include <iomanip>
#include <cstring>
#include <errno.h>
#include "srlogger.h"
#include "srnetmqtt.h"

using namespace std;

#define EMQTT_BASE CURL_LAST
#define EMQTT_PROTOVER (EMQTT_BASE + 1)
#define EMQTT_ID (EMQTT_PROTOVER + 1)
#define EMQTT_SERVER (EMQTT_ID + 1)
#define EMQTT_TOKEN (EMQTT_SERVER + 1)
#define EMQTT_AUTH (EMQTT_TOKEN + 1)
#define EMQTT_PACKET (EMQTT_AUTH + 1)
#define EMQTT_SERIAL (EMQTT_PACKET + 1)
#define EMQTT_DESERIAL (EMQTT_SERIAL + 1)
#define EMQTT_LAST (EMQTT_DESERIAL + 1)

static const char* const emsg[] =
{
    "OK!", "unacceptable protocol version", "client id rejected",
    "server unavailable", "bad user name or password", "not authorized",
    "invalid packet", "serialization error", "deserialization error"
};

#define MQTT_SERIALIZE_BUFFER_SIZE 4096


SrNetMqtt::SrNetMqtt(const string &id, const string &server) :
        SrNetSocket(server), client(id), pval(0), wqos(0), iswill(), wretain(), isuser(), ispass()
{
}

void SrNetMqtt::setKeepalive(int val)
{
    pval = val;

    srDebug("MQTT: set keepalive to " + to_string(val));
}

int SrNetMqtt::connect(bool clean, char nflag)
{
    (void) nflag;

    if (SrNetSocket::connect() == -1)
    {
        return -1;
    }

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.keepAliveInterval = pval;
    data.MQTTVersion = 4;
    data.clientID.cstring = (char*)client.c_str();
    data.cleansession = clean;

    if (isuser)
    {
        data.username.cstring = (char*)user.c_str();
    }

    if (ispass)
    {
        data.password.cstring = (char*)pass.c_str();
    }

    if (iswill)
    {
        data.will.topicName.cstring = (char*)wtopic.c_str();
        data.will.message.cstring = (char*)wmsg.c_str();
        data.will.qos = wqos;
        data.will.retained = wretain;
    }

    unsigned char buf[200] = { 0 }, sp = 0, rc = 0;
    int len = MQTTSerialize_connect(buf, sizeof(buf), &data);

    srInfo("MQTT: client-id " + client);

    if (sendBuf((const char*) buf, len) <= 0)
    {
        return -1;
    }

    const size_t offset = resp.size();
    len = recv(SR_SOCK_RXBUF_SIZE);
    if (len <= 0)
    {
        return -1;
    }

    unsigned char* const ptr = (unsigned char*) resp.c_str() + offset;
    const int ret = MQTTDeserialize_connack(&sp, &rc, ptr, resp.size());

    if (ret == 1 && rc)
    {
        errNo = EMQTT_BASE + rc;
        strcpy(_errMsg, emsg[rc]);

        srError(string("MQTT: ") + emsg[rc]);
    }

    clock_gettime(CLOCK_MONOTONIC_COARSE, &t0);

    return ret == 1 && rc == 0 ? 0 : -1;
}

int SrNetMqtt::publish(const string &topic, const string &msg, char nflag)
{
    const int qos = (nflag >> 1) & 3;

    srDebug("MQTT pub: " + topic + '@' + to_string(qos) + ":\n" + msg);

    unsigned char buf[100] = { 0 };
    unsigned char *ptr = buf;
    *ptr++ = 0x30 | nflag;
    const int remlen = 2 + topic.size() + (qos ? 2 : 0) + msg.size();

    ptr += MQTTPacket_encode(ptr, remlen);
    writeCString(&ptr, topic.c_str());

    if (qos)
    {
        writeInt(&ptr, 1);
    }

    if (sendBuf((const char *) buf, ptr - buf) != ptr - buf)
    {
        return -1;
    }

    size_t len = msg.size();
    const char *pch = msg.c_str();

    for (size_t i = 0; i < len;)
    {
        const int n = sendBuf(pch + i, len - i);
        if (n == -1)
        {
            return -1;
        }

        i += n;
    }

    errno = errNo = 0;
    if (qos)
    {
        len = recv(SR_SOCK_RXBUF_SIZE);
    }

    if (len <= 0)
    {
        srError(string("MQTT pub: ") + _errMsg);

        return -1;
    } else
    {
        return 0;
    }
}

static int sub(SrNetMqtt *mqtt, MQTTString *ts, int *qos, int n, char *errbuf)
{
    if (srLogIsEnabledFor(SRLOG_INFO))
    {
        string debug;
        for (auto i = 0; i < n; ++i)
        {
            debug += ts[i].cstring;
            debug += '@' + to_string(qos[i]) + ' ';
        }

        srInfo("MQTT sub: " + debug);
    }

    unsigned char buf[MQTT_SERIALIZE_BUFFER_SIZE] = { 0 };
    int len = MQTTSerialize_subscribe(buf, sizeof(buf), 0, 1, n, ts, qos);
    if (len <= 0)
    {
        mqtt->errNo = EMQTT_SERIAL;
        strcpy(errbuf, emsg[EMQTT_SERIAL - EMQTT_BASE]);

        srError(string("MQTT sub: ") + errbuf);

        return -1;
    }

    for (int i = 0; i < len;)
    {
        const int n = mqtt->sendBuf((const char*) buf + i, len - i);
        if (n <= 0)
        {
            return -1;
        }

        i += n;
    }

    const size_t offset = mqtt->response().size();
    if (mqtt->recv(SR_SOCK_RXBUF_SIZE) <= 0)
    {
        srError(string("MQTT sub: ") + mqtt->errMsg());
        return -1;
    }

    int c = 0;
    unsigned short packet = 0;
    unsigned char *pch = (unsigned char*) mqtt->response().c_str() + offset;
    len = mqtt->response().size() - offset;

    const int ret = MQTTDeserialize_suback(&packet, n, &c, qos, pch, len) - 1;
    if (ret)
    {
        mqtt->errNo = EMQTT_DESERIAL;
        strcpy(errbuf, emsg[EMQTT_DESERIAL - EMQTT_BASE]);

        srError(string("MQTT sub: ") + errbuf);
    } else if (srLogIsEnabledFor(SRLOG_INFO))
    {
        string debug;
        for (auto i = 0; i < c; ++i)
        {
            debug += to_string(qos[i]) + ' ';
        }

        srInfo("MQTT suback: " + debug);
    }

    return ret;
}

static int unsub(SrNetMqtt *mqtt, MQTTString *ts, int n, char *errbuf)
{
    if (srLogIsEnabledFor(SRLOG_DEBUG))
    {
        string debug;
        for (auto i = 0; i < n; ++i)
        {
            debug += ts[i].cstring;
            debug += ' ';
        }

        srDebug("MQTT unsub: " + debug);
    }

    unsigned char buf[MQTT_SERIALIZE_BUFFER_SIZE] = { 0 };
    int len = MQTTSerialize_unsubscribe(buf, sizeof(buf), 0, 1, n, ts);
    if (len <= 0)
    {
        mqtt->errNo = EMQTT_SERIAL;
        strcpy(errbuf, emsg[EMQTT_SERIAL - EMQTT_BASE]);

        srError(string("MQTT unsub: ") + errbuf);

        return -1;
    }

    for (int i = 0; i < len;)
    {
        const int n = mqtt->sendBuf((const char*) buf + i, len - i);
        if (n <= 0)
        {
            return -1;
        }

        i += n;
    }

    const size_t offset = mqtt->response().size();
    if (mqtt->recv(SR_SOCK_RXBUF_SIZE) <= 0)
    {
        srError(string("MQTT unsub: ") + mqtt->errMsg());
        return -1;
    }

    unsigned short packet = 0;
    unsigned char *ptr = (unsigned char*) mqtt->response().c_str() + offset;
    n = mqtt->response().size() - offset;

    return MQTTDeserialize_unsuback(&packet, ptr, n) - 1;
}

int SrNetMqtt::subscribe(const string &topic, int *qos, char nflag)
{
    (void) nflag;
    MQTTString ts = MQTTString_initializer;
    ts.cstring = (char*) topic.c_str();

    return sub(this, &ts, qos, 1, this->_errMsg);
}

int SrNetMqtt::subscribe(const char *topics[], int *qos, int count, char nflag)
{
    (void) nflag;
    MQTTString ts[100] = { MQTTString_initializer };
    const int len = count < 100 ? count : 100;

    for (int i = 0; i < len; ++i)
    {
        ts[i].cstring = (char*) topics[i];
    }

    return sub(this, ts, qos, len, this->_errMsg);
}

int SrNetMqtt::subscribe(const string topics[], int *qos, int count, char nflag)
{
    (void) nflag;
    MQTTString ts[100] = { MQTTString_initializer };
    const int len = count < 100 ? count : 100;

    for (int i = 0; i < len; ++i)
    {
        ts[i].cstring = (char*) topics[i].c_str();
    }

    return sub(this, ts, (int*) qos, len, this->_errMsg);
}

int SrNetMqtt::unsubscribe(const string &topic, char nflag)
{
    (void) nflag;
    MQTTString ts = MQTTString_initializer;
    ts.cstring = (char*) topic.c_str();

    return unsub(this, &ts, 1, this->_errMsg);
}

int SrNetMqtt::unsubscribe(const char *topics[], int count, char nflag)
{
    (void) nflag;
    MQTTString ts[100] = { MQTTString_initializer };
    const int len = count < 100 ? count : 100;

    for (int i = 0; i < len; ++i)
    {
        ts[i].cstring = (char*) topics[i];
    }

    return unsub(this, ts, len, this->_errMsg);
}

int SrNetMqtt::unsubscribe(const string topics[], int count, char nflag)
{
    (void) nflag;
    MQTTString ts[100] = { MQTTString_initializer };
    const int len = count < 100 ? count : 100;

    for (int i = 0; i < len; ++i)
    {
        ts[i].cstring = (char*) topics[i].c_str();
    }

    return unsub(this, ts, len, this->_errMsg);
}

int SrNetMqtt::disconnect(char nflag)
{
    (void) nflag;
    unsigned char buf[200];
    const int len = MQTTSerialize_disconnect(buf, sizeof(buf));

    return sendBuf((const char*) buf, len) == len ? 0 : -1;
}

static int _schedule(unsigned char *buf, int len, int &qos, uint16_t &packet, SrMqttAppMsg &msg)
{
    MQTTString ts = MQTTString_initializer;
    unsigned char dup = 0, retain = 0, *payload = NULL;
    int n = 0;

    if (MQTTDeserialize_publish(&dup, &qos, &retain, &packet, &ts, &payload, &n, buf, len) != 1)
    {
        return -1;
    }
    else if (buf + len < payload + n)
    {
        return 0;
    }

    msg.topic.assign((char *) ts.lenstring.data, ts.lenstring.len);
    msg.data.assign((char*) payload, n);

    return payload - buf + n;
}

int SrNetMqtt::yield(int ms)
{
    timespec now;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &now);

    if (pval && t0.tv_sec + pval <= now.tv_sec)
    {
        if (ping() == -1)
        {
            return -1;
        }

        t0.tv_sec = now.tv_sec;
    } else
    {
        const int mysec = this->t;
        this->t = ms < 1000 ? 1 : ms / 1000;
        recv(SR_SOCK_RXBUF_SIZE);

        this->t = mysec;

        if (resp.empty())
        {
            return 0;
        }
    }

    auto lm = [](const _Item &l, const string &r) { return l.first < r; };

    int i = 0;
    bool finish = false;
    const int len = resp.size();
    SrMqttAppMsg msg;

    for (int n = 0; i < len; i += n)
    {
        unsigned char* const buf = (unsigned char*) resp.c_str() + i;
        const char type = ((*buf) & 0xf0) >> 4;

        switch (type)
        {
            case 3:
            {
                int qos = 0;
                unsigned short packet = 0;
                n = _schedule(buf, len - i, qos, packet, msg);

                if (n == -1)
                {
                    errNo = EMQTT_DESERIAL;
                    strcpy(_errMsg, emsg[errNo - EMQTT_BASE]);
                    srWarning(string("MQTT recv: ") + _errMsg);

                    break;
                } else if (n == 0)
                {
                    finish = true;
                    break;
                }

                if (qos == 1)
                {
                    unsigned char pb[10] = { 0 };
                    const int pl = MQTTSerialize_puback(pb, 10, packet);
                    sendBuf((char*) pb, pl);
                }

                srDebug("MQTT appmsg: " + msg.topic + '@' + to_string(qos) + ": " + msg.data);

                auto beg = hdls.begin();
                auto end = hdls.end();
                auto it = lower_bound(beg, end, msg.topic, lm);
                if (it != hdls.end() && it->first == msg.topic)
                {
                    (*it->second)(msg);
                }

                break;
            }

            case 2:                // connack
            case 4:                // puback
            case 5:                // pubrec
            case 6:                // pubrel
            case 7:                // pubcomp
            case 11:
            {
                n = 4;
                break; // unsuback
            }

            case 9:
            {
                ++i;         // suback
                i += MQTTPacket_decodeBuf(buf + 1, &n);
                break;
            }

            case 13:
            {
                n = 2;
                break; // pingresp
            }

            default:
            {
                srWarning("MQTT recv: type " + to_string((int) type));

                n = 1;
            }
        }

        if (finish)
        {
            break;
        }
    }

    resp.erase(0, i);

    return 0;
}

int SrNetMqtt::ping(char nflag)
{
    (void) nflag;
    unsigned char buf[20] = { 0 };

    srDebug("MQTT: ping");

    const int len = MQTTSerialize_pingreq(buf, sizeof(buf));
    if (sendBuf((const char*) buf, len) <= 0)
    {
        return -1;
    }

    errno = errNo = 0;
    const int ret = recv(SR_SOCK_RXBUF_SIZE);
    if (ret <= 0)
    {
        srError("MQTT: ping timeout.");

        return -1;
    }

    return 0;
}
