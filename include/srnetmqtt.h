#ifndef SRNETMQTT_H
#define SRNETMQTT_H
#include <string>
#include <algorithm>
#include <vector>
#include "../ext/pahomqtt/MQTTPacket/src/MQTTPacket.h"
#include "srnetsocket.h"


struct SrMqttAppMsg
{
        SrMqttAppMsg(const std::string &t, const std::string &s):
                topic(t), data(s){}
        const std::string topic;
        const std::string data;
};


class SrMqttAppMsgHandler
{
public:
        SrMqttAppMsgHandler() {}
        virtual ~SrMqttAppMsgHandler() {}
        virtual void operator()(const SrMqttAppMsg&) = 0;
};


class SrNetMqtt: public SrNetSocket
{
public:
        using string = std::string;
        SrNetMqtt(const string &clientid, const string &server);
        virtual ~SrNetMqtt() {}
        int connect(bool cleanSession = true, char hflag = 0);
        int publish(const string &topic, const string &msg, char hflag = 0);
        int subscribe(const string &topic, int qos, char hflag = 2,
                      int *gqos = NULL);
        int subscribe(const char *topics[], const int *qos, int count,
                      char hflag = 2, int *gqos = NULL);
        int subscribe(const string topics[], const int *qos, int count,
                      char hflag = 2, int *gqos = NULL);
        int unsubscribe(const string &topic, char hflag = 0);
        int unsubscribe(const char *topics[], int count, char hflag = 0);
        int unsubscribe(const string topics[], int count, char hflag = 0);
        int disconnect(char hflag = 0);
        int ping(char hflag = 0);
        int yield(int ms);
        void addMsgHandler(const string &t, SrMqttAppMsgHandler *mh) {
                auto pred = [](const string &lhs, const _Item &rhs) {
                        return lhs < rhs.first;
                };
                const auto it = upper_bound(hdls.begin(), hdls.end(), t, pred);
                hdls.insert(it, make_pair(t, mh));
        }
        void setUsername(const char *username) {
                user = username ? username : "";
                isuser = username != NULL;
        }
        void setPassword(const char *password) {
                pass = password ? password : "";
                ispass = password != NULL;
        }
        void setWill(const char *topic, const char *msg, int qos, bool retain) {
                wtopic = topic ? topic : "";
                wmsg = msg ? msg : "";
                iswill = topic != NULL;
                wqos = qos;
                wretain = retain;
        }
        void setKeepalive(int val);
        int keepalive() const {return pval;}
private:
        typedef std::pair<string, SrMqttAppMsgHandler*> _Item;
        std::vector<_Item> hdls;
        string client;
        string user;
        string pass;
        string wtopic;
        string wmsg;
        struct timespec t0;
        uint16_t pval;
        uint8_t wqos;
        bool iswill;
        bool wretain;
        bool isuser;
        bool ispass;
};

#endif /* SRNETMQTT_H */
