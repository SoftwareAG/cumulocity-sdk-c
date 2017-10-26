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

#ifndef SRREPORTER_H
#define SRREPORTER_H

#include <memory>
#include "srtypes.h"
#include "srnethttp.h"
#include "srnetmqtt.h"
#include "srqueue.h"
#include "srlogger.h"

#define SR_MQTTOPT_KEEPALIVE 1

/**
 *  \class SrReporter
 *  \brief The reporter thread for sending all requests to Cumulocity.
 *
 *  The SrReporter is responsible for sending all requests (measurements,
 *  alarms, events, etc.) to Cumulocity. For traffic saving, the SrReporter
 *  implements request aggregation when there are consecutive requests in the
 *  queue within less than SR_REPORTER_VAL (default 400) milliseconds. It also
 *  implements a multiple retry and exponential waiting mechanism for
 *  counteracting the instability of mobile networks. Additionally, it
 *  implements a capacity limited buffering technique for counteracting long
 *  period of network error. This buffering technique can be further categorized
 *  into memory backed and file backed buffering. Memory backed buffering is
 *  more efficient since there is no file I/O involved, while the buffering is
 *  limited by the available memory and doesn't survive a sudden outage.
 *  Oppositely, file backed buffering performs a lot of file I/O operations,
 *  but at the same time, its capacity is much larger and buffered messages will
 *  not be lost in case of sudden outage.
 */
class _Pager;

class SrReporter
{
private:
    using string = std::string;
public:
    /**
     *  \brief SrReporter HTTP constructor.
     *
     *  Construct a SrReporter instance which uses SrNetHttp as underlying
     *  network stack.
     *
     *  \param server Cumulocity server URL (no trailing slash).
     *  \param xid eXternal ID of the registered SmartREST template.
     *  \param auth authentication token from SrAgent.
     *  \param out reference to the SrAgent egress queue.
     *  \param in reference to the SrAgent ingress queue.
     *  \param cap capacity of the request buffer.
     *  \param buffile file name for file-backed buffering. Default is
     *  memory backed buffering. Set it to a non empty string enable
     *  file backed buffering.
     */
    SrReporter(const string &server, const string &xid, const string &auth,
            SrQueue<SrNews> &out, SrQueue<SrOpBatch> &in, uint16_t cap = 1000,
            const string buffile = "");
    /**
     *  \brief SrReporter MQTT constructor.
     *
     *  Construct a SrReporter instance which uses SrNetMqtt as underlying
     *  network stack.
     *
     *  \param server Cumulocity server URL. Use correct port for MQTT,
     *  i.e., 1883 for plain MQTT, 8883 for MQTT + TLS by standard. For
     *  enabling TLS, you still need to prepend the URL with https.
     *  \param deviceId device ID, same as SrAgent.deviceID().
     *  \param xid SmartREST template IXD.
     *  \param user MQTT Connect username, should be SrAgent.tenant() +
     *  '/' + SrAgent.username().
     *  \param pass MQTT Connect pass, should be SrAgent.password().
     *  \param out reference to the SrAgent egress queue.
     *  \param in reference to the SrAgent ingress queue.
     *  \param cap capacity of the request buffer.
     *  \param buffile file name for file-backed buffering. Default is
     *  memory backed buffering. Set it to a non empty string enable
     *  file backed buffering.
     */
    SrReporter(const string &server, const string &deviceId, const string &xid,
            const string &user, const string &pass, SrQueue<SrNews> &out,
            SrQueue<SrOpBatch> &in, uint16_t cap = 1000, const string buffile =
                    "");
    virtual ~SrReporter();

    /**
     *  \brief Get the current capacity of the request buffer.
     *
     *  capacity signifies the capacity of the underlying buffering
     *  mechanism. For memory backed buffering, it signifies the number
     *  of messages can be buffered. For file backed buffering, it
     *  signifies the number of file pages available.
     *
     *  Messages with SR_PRIO_BUF bit set will be buffered when the network
     *  is unavailable. However, when the network is down for longer time,
     *  and the number of messages with SR_PRIO_BUF bit set exceeds the
     *  defined capacity, older messages will be discarded.

     *  \note Message buffering is intended for messages with higher demand
     *  of reliability because of temporary network error. Abuse of
     *  SR_PRIO_BUF bit by setting it for all messages will not only
     *  downgrade agent's performance, but also real important messages
     *  will often be discarded.
     *
     *  \note The actual buffered messages will often be less than capacity
     *  because auxiliary messages for setting XIDs will also take space.
     *  Therefore, it's sensible not to use many SmartREST templates. For
     *  file backed buffering, page fragmentation will also waste a
     *  fraction of the capacity.
     */
    uint16_t capacity() const;
    /**
     *  \brief Set the capacity of the request buffer.
     *
     *  If the new capacity is smaller than already buffered messages,
     *  already buffered messages will NOT be discarded immediately,
     *  rather, they are only guaranteed to be eventually discarded or
     *  successfully sent, and the buffer capacity will eventually
     *  converges to the new capacity.
     *
     *  \param cap new buffer capacity.
     */
    void setCapacity(uint16_t cap);
    /**
     *  \brief Start the SrReporter thread.
     *
     *  \return 0 for success, failure otherwise.
     */
    int start();
    /**
     *  \brief Check if the SrReporter is sleeping.
     *  \return true if sleeping, false otherwise.
     */
    bool isSleeping() const
    {
        return sleeping;
    }
    /**
     *  \brief Put the SrReporter to sleep.
     *
     *  SrNews with SR_PRIO_BUF bit set will be buffered, other messages
     *  will be discarded. Sleeping an already slept SrReporter has no
     *  effect.
     */
    void sleep()
    {
        sleeping = true;
        srNotice("reporter: slept.");
    }
    /**
     *  \brief Resume a sleeping SrReporter. Resume a non-sleeping
     *  SrReporter has no effect.
     */
    void resume()
    {
        sleeping = false;
        srNotice("reporter: resumed.");
    }
    /**
     *  \brief Set various options for MQTT.
     *
     *  Supported MQTT option list:
     *
     *  - SR_MQTTOPT_KEEPALIVE [E]: MQTT keepalive interval in seconds.
     *
     *  \param option various MQTT options.
     *  \param parameter value for corresponding MQTT option.
     *
     *  \note Options marked with [E] must be set before calling start(),
     *  as they would otherwise has no effect.
     */
    void mqttSetOpt(int option, long parameter);

protected:
    /**
     *  \brief pthread routine function.
     *  \param arg pointer to a SrReporter instance.
     */
    static void *func(void *arg);

private:

    std::unique_ptr<SrNetHttp> http;
    std::unique_ptr<SrNetMqtt> mqtt;
    SrQueue<SrNews> &out;
    SrQueue<SrOpBatch> &in;
    const string &xid;
    std::unique_ptr<_Pager> ptr;
    bool sleeping;
    bool isfilebuf;
    pthread_t tid;
};

#endif /* SRREPORTER_H */
