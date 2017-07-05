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

#ifndef SRNETMQTT_H
#define SRNETMQTT_H

#include <string>
#include <algorithm>
#include <vector>
#include "../ext/pahomqtt/MQTTPacket/src/MQTTPacket.h"
#include "srnetsocket.h"

/**
 *  \class SrMqttAppMsg
 *  \brief MQTT application message received from the server.
 */
struct SrMqttAppMsg
{
    /**
     *  \brief SrMqttAppMsg default constructor.
     */
    SrMqttAppMsg()
    {
    }
    /**
     *  \brief SrMqttAppMsg constructor.
     *
     *  \param t topic name.
     *  \param s application message data.
     */
    SrMqttAppMsg(const std::string &t, const std::string &s) :
            topic(t), data(s)
    {
    }
    /**
     *  \brief Topic name.
     */
    std::string topic;
    /**
     *  \brief Payload data.
     */
    std::string data;
};

/**
 *  \class SrMqttAppMsgHandler
 *  \brief Virtual abstract functor for SrMqttAppMsg callback handler.
 */
class SrMqttAppMsgHandler
{
public:
    /**
     *  \brief SrMqttAppMsgHandler default constructor.
     */
    SrMqttAppMsgHandler()
    {
    }

    virtual ~SrMqttAppMsgHandler()
    {
    }
    /**
     *  \brief Callback function to be invoked when data received from
     *  registered message topic.
     *
     *  This is an abstract virtual function. For use, subclass
     *  SrMqttAppMsgHandler and implement your own version of operator().
     *
     *  \param appmsg received MQTT application message.
     */
    virtual void operator()(const SrMqttAppMsg &appmsg) = 0;
};

/**
 *  \class SrNetMqtt
 *  \brief Generic MQTT network stack implementation.
 *
 *  Implementation is based on MQTT specification v3.1.1
 *  (http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/mqtt-v3.1.1.html). It
 *  supports both plain MQTT and MQTT + TLS.
 *
 *  \note Currently only QoS level 0 and 1 are implemented, QoS 2 is not yet
 *  supported for both PUBLISH and SUBSCRIBE.
 */
class SrNetMqtt: public SrNetSocket
{
public:

    using string = std::string;
    /**
     *  \brief SrNetMqtt constructor.
     *
     *  Construct a SrNetMqtt instance.
     *
     *  \param clientid MQTT Client Identifier used in CONNECT packet.
     *  \param server MQTT server URL.
     *
     *  \note server URL can be of form  http://www.example.com:1883 or
     *  https://www.example.com:8883. The client can figure out to use
     *  whether plain MQTT or MQTT + TLS based on the protocol name, i.e.
     *  http/https. Hence, you must specify the correct port number.
     */
    SrNetMqtt(const string &clientid, const string &server);

    virtual ~SrNetMqtt()
    {
    }
    /**
     *  \brief Establish connection to a MQTT server.
     *
     *  The method establishes first the underlying TCP connection and then
     *  the MQTT connection by sending an MQTT CONNECT packet via this
     *  connection. If MQTT + TLS is used, the method also takes care of
     *  TLS handshake and certificate verification.
     *
     *  \param cleanSession MQTT Connect flag Clean Session
     *  \param hflag nibble flag in MQTT fixed header.
     */
    int connect(bool cleanSession = true, char hflag = 0);

    /**
     *  \brief Publish message \a msg to topic \a topic.
     *
     *  \param topic topic name to be published to.
     *  \param msg application message for publishing.
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int publish(const string &topic, const string &msg, char hflag = 0);

    /**
     *  \brief Subscribe to topic filter \a topic with QoS level \a qos.
     *
     *  \param topic topic filter for subscribing.
     *  \param qos pointer to QoS level.
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     *
     *  \note on success, all subscribe() methods update the qos array with
     *  actual granted QoS from the server, this may differ from the your
     *  requested QoS levels. A subscription can also be rejected with
     *  failure code 0x80. To make sure a subscription really has effect,
     *  you must check the \a qos array after successful return.
     */
    int subscribe(const string &topic, int *qos, char hflag = 2);

    /**
     *  \brief Subscribe to an array of topic filter \a topics with QoS
     *  level \a qos.
     *
     *  \param topics topic filter array for subscribing.
     *  \param qos QoS level array.
     *  \param count number of topic filters and QoS.
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int subscribe(const char *topics[], int *qos, int count, char hflag = 2);

    /**
     *  \brief Subscribe to an array of topic filter \a topics with QoS
     *  level \a qos.
     *
     *  \param topics topic filter array for subscribing.
     *  \param qos QoS level array.
     *  \param count number of topic filters and QoS.
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int subscribe(const string topics[], int *qos, int count, char hflag = 2);
    /**
     *  \brief Unsubscribe topic \a topic from server.
     *
     *  \param topic topic filter for unsubscribing.
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int unsubscribe(const string &topic, char hflag = 0);

    /**
     *  \brief Unsubscribe an array of topic filter \a topics from server.
     *
     *  \param topics topic filter array for unsubscribing.
     *  \param count number of topic filters.
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int unsubscribe(const char *topics[], int count, char hflag = 0);

    /**
     *  \brief Unsubscribe an array of topic filter \a topics from server.
     *
     *  \param topics topic filter array for unsubscribing.
     *  \param count number of topic filters.
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int unsubscribe(const string topics[], int count, char hflag = 0);
    /**
     *  \brief Disconnect from MQTT server.
     *
     *  Send MQTT DISCONNECT packet to the server.
     *
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int disconnect(char hflag = 0);

    /**
     *  \brief Send MQTT PINGREQ packet to server.
     *
     *  \param hflag nibble flag in MQTT fixed header.
     *  \return 0 on success, -1 on failure.
     */
    int ping(char hflag = 0);

    /**
     *  \brief Draft horse method ought to be called periodically.
     *
     *  This method tried to recv() from the server with timeout \a ms
     *  milliseconds, and invoke any registered SrMqttAppMsgHandler if
     *  there is data received from corresponding topic. It also clears
     *  other control packets (e.g. SUBACK, PUBACK) in the receive buffer.
     *  In addition, it checks for the keep-alive interval expiration, and
     *  will ping() the server by sending MQTT PINGREQ control packet.
     *
     *  \param ms recv() timeout in milliseconds.
     *  \return 0 on success, -1 on failure.
     *
     *  \note The method returns 0 if recv() timed out. -1 indicates a
     *  network error, e.g., broken connection.
     *
     *  \note Because of current implementation limit, timeout \a ms
     *  has only second resolution. For example, both 200 ms and 1234 ms
     *  will be rounded to 1 second.
     */
    int yield(int ms);

    /**
     *  \brief Add application message handler \a mh to topic \a t.
     *
     *  The added SrMqttMsgHandler \a mh will be invoked automatically
     *  the next time yield() is called after server published an
     *  application message to the corresponding topic \a t.
     *
     *  \param t topic name. Wildcard filters are not supported.
     *  \param mh SrMqttAppMsgHandler subclass instance. NULL to clear
     *  an existing handler.
     *  \return return type
     */
    void addMsgHandler(const string &t, SrMqttAppMsgHandler *mh)
    {
        auto pred = [](const string &lhs, const _Item &rhs)
        {
            return lhs < rhs.first;
        };
        const auto it = upper_bound(hdls.begin(), hdls.end(), t, pred);
        hdls.insert(it, make_pair(t, mh));
    }

    /**
     *  \brief Set username for MQTT CONNECT packet.
     *
     *  \param username user name for authentication. Use NULL will
     *  effectively set User Name Flag in Connect Flags to 0.
     */
    void setUsername(const char *username)
    {
        user = username ? username : "";
        isuser = (username != NULL) ? true : false;
    }

    /**
     *  \brief Set password for MQTT CONNECT packet.
     *
     *  \param password password for authentication. Use NULL will
     *  effectively set Password Flag in Connect Flags to 0.
     */
    void setPassword(const char *password)
    {
        pass = password ? password : "";
        ispass = (password != NULL) ? true : false;
    }

    /**
     *  \brief Set Will Message for MQTT CONNECT packet.
     *
     *  \param topic Will Topic. NULL will effectively clear Will Flag.
     *  \param msg Will Message.
     *  \param qos Will QoS.
     *  \param retain Will Retain.
     */
    void setWill(const char *topic, const char *msg, int qos, bool retain)
    {
        wtopic = topic ? topic : "";
        wmsg = msg ? msg : "";
        iswill = (topic != NULL) ? true : false;
        wqos = qos;
        wretain = retain;
    }

    /**
     *  \brief Set keep-alive interval for MQTT CONNECT packet.
     *
     *  \param val keep-alive interval in seconds. 0 to disable MQTT ping
     *  mechanism, negative values are undefined behavior.
     *
     *  \note Since keep-alive interval is sent in the MQTT Connect packet,
     *  you must call this method before connect() or do a re-connect,
     *  otherwise you may experience strange behavior when using yield().
     */
    void setKeepalive(int val);

    /**
     *  \brief Get last set keep-alive interval.
     *
     *  \return keep-alive interval in seconds.
     */
    int keepalive() const
    {
        return pval;
    }

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
