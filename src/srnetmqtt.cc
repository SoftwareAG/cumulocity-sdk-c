#include <sstream>
#include <iomanip>
#include <cstring>
#include <errno.h>
#include <memory>
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

static const char* emsg[] = {
        "OK!", "unacceptable protocol version", "client id rejected",
        "server unavailable", "bad user name or password", "not authorized",
        "invalid packet", "serialization error", "deserialization error"
};


SrNetMqtt::SrNetMqtt(const string &id, const string &server):
        SrNetSocket(server), client(id) ,pval(0), wqos(0),
        iswill(), wretain(), isuser(), ispass()
{
        clock_gettime(CLOCK_MONOTONIC_COARSE, &t0);
}

void SrNetMqtt::setKeepalive(int val)
{
        pval = val;
        srDebug("MQTT: set keepalive to " + to_string(val));
}


int SrNetMqtt::connect(bool clean, char nflag)
{
        (void)nflag;
        if (SrNetSocket::connect() == -1)
                return -1;

        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        data.keepAliveInterval = pval;
        data.MQTTVersion = 4;
        data.clientID.cstring = (char*)client.c_str();
        data.cleansession = clean;
        if (isuser)
                data.username.cstring = (char*)user.c_str();
        if (ispass)
                data.password.cstring = (char*)pass.c_str();
        if (iswill) {
                data.will.topicName.cstring = (char*)wtopic.c_str();
                data.will.message.cstring = (char*)wmsg.c_str();
                data.will.qos = wqos;
                data.will.retained = wretain;
        }
        unsigned char buf[200], sp, rc;
        int len = MQTTSerialize_connect(buf, sizeof(buf), &data);
        srInfo("MQTT: connect as " + client);
        if (sendBuf((const char*)buf, len) <= 0)
                return -1;
        len = recv(SR_SOCK_RXBUF_SIZE);
        if (len <= 0)
                return -1;
        unsigned char *ptr = (unsigned char*)resp.c_str();
        const int ret = MQTTDeserialize_connack(&sp, &rc, ptr, resp.size());
        if (rc) {
                errNo = EMQTT_BASE + rc;
                strcpy(_errMsg, emsg[rc]);
                srError(string("MQTT: ") + emsg[rc]);
        }
        return ret == 1 && rc == 0 ? 0 : -1;
}


int SrNetMqtt::publish(const string &topic, const string &msg, char nflag)
{
        MQTTString ts = MQTTString_initializer;
        ts.cstring = (char*)topic.c_str();
        const size_t size = topic.size() + msg.size() + 10;
        const int qos = (nflag >> 1) & 3;
        const unsigned char dup = nflag >> 3, retain = nflag & 1;
        const unsigned short packet = 1;
        unique_ptr<unsigned char[]> buf(new unsigned char[size]);
        unsigned char *p = (unsigned char*)msg.c_str();
        srDebug("MQTT pub: " + topic + '@' + to_string(qos) + ": " + msg);
        int len = MQTTSerialize_publish(buf.get(), size, dup, qos, retain,
                                        packet, ts, p, msg.size());
        len = sendBuf((const char*)buf.get(), len);
        if (len <= 0) return -1;
        errno = errNo = 0;
        if (qos)
                len = recv(SR_SOCK_RXBUF_SIZE);
        if (len <= 0) {
                srError(string("MQTT pub: ") + _errMsg);
                return -1;
        } else {
                return 0;
        }
}


static int sub(SrNetMqtt *mqtt, MQTTString *ts, const int *qos, int n, int *G,
               char *errbuf)
{
        if (srLogIsEnabledFor(SRLOG_INFO)) {
                string debug;
                for (auto i = 0; i < n; ++i) {
                        debug += ts[i].cstring;
                        debug += '@' + to_string(qos[i]) + ' ';
                }
                srInfo("MQTT sub: " + debug);
        }
        // count for every topic maximum 100 bytes
        unique_ptr<unsigned char[]> buf(new unsigned char[n * 100]);
        int len = MQTTSerialize_subscribe(buf.get(), n * 100, 0, 1, n,
                                          ts, (int*)qos);
        if (len <= 0) {
                mqtt->errNo = EMQTT_SERIAL;
                strcpy(errbuf, emsg[EMQTT_SERIAL - EMQTT_BASE]);
                srError(string("MQTT sub: ") + errbuf);
                return -1;
        }
        if (mqtt->sendBuf((const char*)buf.get(), len) <= 0)
                return -1;
        if (mqtt->recv(SR_SOCK_RXBUF_SIZE) <= 0) {
                srError(string("MQTT sub: ") + mqtt->errMsg());
                return -1;
        }
        int c, *ptr = G;
        unique_ptr<int []> gq;
        if (G == NULL) {
                gq.reset(new int[n]);
                ptr = gq.get();
        }
        unsigned short packet = 0;
        unsigned char *pch = (unsigned char*)mqtt->response().c_str();
        len = mqtt->response().size();
        int ret = MQTTDeserialize_suback(&packet, n, &c, ptr, pch, len) - 1;
        if (ret) {
                mqtt->errNo = EMQTT_DESERIAL;
                strcpy(errbuf, emsg[EMQTT_DESERIAL - EMQTT_BASE]);
                srError(string("MQTT sub: ") + errbuf);
        } else if (srLogIsEnabledFor(SRLOG_DEBUG)) {
                string debug;
                for (auto i = 0; i < c; ++i)
                        debug += to_string(ptr[i]) + ' ';
                srDebug("MQTT suback: " + debug);
        }
        return ret;
}


static int unsub(SrNetMqtt *mqtt, MQTTString *ts, int n, char *errbuf)
{
        if (srLogIsEnabledFor(SRLOG_DEBUG)) {
                string debug;
                for (auto i = 0; i < n; ++i) {
                        debug += ts[i].cstring;
                        debug += ' ';
                }
                srDebug("MQTT unsub: " + debug);
        }
        // count for every topic maximum 100 bytes
        unique_ptr<unsigned char[]> buf(new unsigned char[n * 100]);
        int len = MQTTSerialize_unsubscribe(buf.get(), n * 100, 0, 1, n, ts);
        if (len <= 0) {
                mqtt->errNo = EMQTT_SERIAL;
                strcpy(errbuf, emsg[EMQTT_SERIAL - EMQTT_BASE]);
                srError(string("MQTT unsub: ") + errbuf);
                return -1;
        }
        if (mqtt->sendBuf((const char*)buf.get(), len) <= 0)
                return -1;
        if (mqtt->recv(SR_SOCK_RXBUF_SIZE) <= 0) {
                srError(string("MQTT unsub: ") + mqtt->errMsg());
                return -1;
        }
        unsigned short packet = 0;
        unsigned char *ptr = (unsigned char*)mqtt->response().c_str();
        n = mqtt->response().size();
        int ret = MQTTDeserialize_unsuback(&packet, ptr, n) - 1;
        return ret;
}


int SrNetMqtt::subscribe(const string &topic, int qos, char nflag, int *gqos)
{
        (void)nflag;
        MQTTString ts = MQTTString_initializer;
        ts.cstring = (char*)topic.c_str();
        return sub(this, &ts, &qos, 1, gqos, this->_errMsg);
}


int SrNetMqtt::subscribe(const char *topics[], const int *qos, int count,
                         char nflag, int *gqos)
{
        (void)nflag;
        unique_ptr<MQTTString[]> ptr(new MQTTString[count]);
        for (int i = 0; i < count; ++i) {
                ptr[i] = MQTTString_initializer;
                ptr[i].cstring = (char*)topics[i];
        }
        return sub(this, ptr.get(), qos, count, gqos, this->_errMsg);
}


int SrNetMqtt::subscribe(const string topics[], const int *qos,
                         int count, char nflag, int *gqos)
{
        (void)nflag;
        unique_ptr<MQTTString[]> ptr(new MQTTString[count]);
        for (int i = 0; i < count; ++i) {
                ptr[i] = MQTTString_initializer;
                ptr[i].cstring = (char*)topics[i].c_str();
        }
        return sub(this, ptr.get(), qos, count, gqos, this->_errMsg);
}


int SrNetMqtt::unsubscribe(const string &topic, char nflag)
{
        (void)nflag;
        MQTTString ts = MQTTString_initializer;
        ts.cstring = (char*)topic.c_str();
        return unsub(this, &ts, 1, this->_errMsg);
}


int SrNetMqtt::unsubscribe(const char *topics[], int count, char nflag)
{
        (void)nflag;
        unique_ptr<MQTTString[]> ptr(new MQTTString[count]);
        for (int i = 0; i < count; ++i) {
                ptr[i] = MQTTString_initializer;
                ptr[i].cstring = (char*)topics[i];
        }
        return unsub(this, ptr.get(), count, this->_errMsg);
}


int SrNetMqtt::unsubscribe(const string topics[], int count, char nflag)
{
        (void)nflag;
        unique_ptr<MQTTString[]> ptr(new MQTTString[count]);
        for (int i = 0; i < count; ++i) {
                ptr[i] = MQTTString_initializer;
                ptr[i].cstring = (char*)topics[i].c_str();
        }
        return unsub(this, ptr.get(), count, this->_errMsg);
}


int SrNetMqtt::disconnect(char nflag)
{
        (void)nflag;
        unsigned char buf[200];
        const int len = MQTTSerialize_disconnect(buf, sizeof(buf));
        return sendBuf((const char*)buf, len) == len ? 0 : -1;
}


int _schedule(unsigned char *buf, int len, int &qos, uint16_t &packet,
              MQTTString *tsp, unsigned char **payload)
{
        unsigned char dup, retain;
        int n;
        int ret = MQTTDeserialize_publish(&dup, &qos, &retain, &packet,
                                          tsp, payload, &n, buf, len);
        return ret == 1 ? n : -1;
}


int SrNetMqtt::yield(int ms)
{
        timespec now;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
        if (pval && t0.tv_sec + pval <= now.tv_sec) {
                t0.tv_sec = now.tv_sec;
                if (ping() == -1) return -1;
        } else {
                int mysec = this->t;
                this->t = ms < 1000 ? 1 : ms / 1000;
                const int ret = recv(SR_SOCK_RXBUF_SIZE);
                this->t = mysec;
                if (ret <= 0) return 0;
        }

        auto lm = [](const _Item &l, const string &r) {return l.first < r;};
        MQTTString ts;
        unsigned short packet;

        int qos, n = 0;
        unsigned char *buf = (unsigned char*)resp.c_str();
        const int len = resp.size();
        if (srLogIsEnabledFor(SRLOG_DEBUG)) {
                ostringstream oss;
                for (const auto &e: resp) {
                        oss << '<' << hex << setfill('0') << setw(2)
                            << int(uint8_t(e)) << '>';
                }
                srDebug("MQTT recv: " + oss.str());
        }
        for (const unsigned char *ptr = buf + len; buf < ptr; buf += n) {
                const char type = ((*buf) & 0xf0) >> 4;
                switch (type) {
                case 3: {
                        unsigned char *pch = buf;
                        n = _schedule(pch, len, qos, packet, &ts, &buf);
                        if (n == -1) {
                                errNo = EMQTT_DESERIAL;
                                strcpy(_errMsg, emsg[errNo - EMQTT_BASE]);
                                srWarning(string("MQTT recv: ") + _errMsg);
                                break;
                        }
                        if (qos == 1) {
                                unsigned char pb[10];
                                int pl = MQTTSerialize_puback(pb, 10, packet);
                                sendBuf((char*)pb, pl);
                        }
                        const char *p1 = (char *)ts.lenstring.data;
                        SrMqttAppMsg msg(string(p1, ts.lenstring.len),
                                         string((char*)buf, n));
                        srDebug("MQTT appmsg: " + msg.topic + '@' +
                                to_string(qos) + ": " + msg.data);
                        auto beg = hdls.begin();
                        auto end = hdls.end();
                        auto it = lower_bound(beg, end, msg.topic, lm);
                        if (it != hdls.end() && it->first == msg.topic)
                                (*it->second)(msg);
                        break;
                }
                case 2:                // connack
                case 4:                // puback
                case 5:                // pubrec
                case 6:                // pubrel
                case 7:                // pubcomp
                case 11: n = 4; break; // unsuback
                case 9: ++buf;         // suback
                        buf += MQTTPacket_decodeBuf(buf, &n); break;
                case 13: n = 2; break; // pingresp
                default: srWarning("MQTT recv: type " + to_string(type));
                        n = 1;
                }
        }
        resp.clear();
        return 0;
}


int SrNetMqtt::ping(char nflag)
{
        (void)nflag;
        unsigned char buf[20];
        srDebug("MQTT: ping.");
        int len = MQTTSerialize_pingreq(buf, sizeof(buf));
         if (sendBuf((const char*)buf, len) <= 0)
                return -1;
        errno = errNo = 0;
        const int ret = recv(SR_SOCK_RXBUF_SIZE);
        return (ret > 0) ? 0 : -1;
}
