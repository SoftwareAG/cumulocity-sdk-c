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

#ifndef SRDEVICEPUSH_H
#define SRDEVICEPUSH_H

#include "srtypes.h"
#include "srqueue.h"
#include "srnethttp.h"
#include "srlogger.h"

/**
 * \class SrDevicePush
 * \brief SmartREST real-time notification implementation.
 *
 *  This class implements the real-time notification in the SmartREST
 *  protocol. It maintains long polling connection to the server using
 *  the bayeux protocol (handshake, subscribe, connect), and block waiting
 *  for the response. To sustain the connection, a single space character
 *  as heartbeat is sent from the server every 10 minutes. When a batch of
 *  operation is received, it redo the connect or handshake based on the
 *  received bayeux advice. The class also features reliable push
 *  (message: 88,batch number) to avoid missing operations due to a
 *  broken connection.
 */
class SrDevicePush
{
public:
    /**
     *  \brief SrDevicePush constructor.
     *  \param server the server URL.
     *  \param xid eXternal ID of the registered SmartREST template.
     *  \param auth Authentication token get from the SrAgent.
     *  \param chn bayeux channel (the managed object ID of the device).
     *  \param queue reference to the ingress queue of the SrAgent.
     */
    SrDevicePush(const std::string &server, const std::string &xid,
            const std::string &auth, const std::string &chn,
            SrQueue<SrOpBatch> &queue);

    virtual ~SrDevicePush()
    {
        pthread_cancel(tid);
    }

    /**
     *  \brief Start device push.
     *
     *  Device push requires a separate thread as it maintains a blocking
     *  connection to the server. This function creates a thread for device
     *  push and starts the real time notification.
     *
     *  \return 0 on success, non-0 if creating thread failed.
     */
    int start();

    /**
     *  \brief Check if device push is sleeping.
     *
     *  \return true if sleeping, false otherwise.
     */
    bool isSleeping() const
    {
        return (bayeuxState == BAYEUX_STATE_IDLE) ? true : false;
    }

    /**
     *  \brief Put device push to sleep.
     *
     *  \note The thread is still running, except the long polling
     *  connection is not maintained anymore, and all received operations
     *  are discarded. sleep an already sleeping device push has no effect.
     */
    void sleep()
    {
        bayeuxState = BAYEUX_STATE_IDLE;

        srNotice("push: sleeping...");
    }

    /**
     *  \brief Resume a formerly sleeping device push.
     *
     *  This function restarts the long polling connection again for
     *  receiving operations. resume a non-sleeping device push has no
     *  effect.
     */
    void resume()
    {
        bayeuxState = BAYEUX_STATE_HANDSHAKE;

        srNotice("push: resuming...");
    }

    /**
     *  \brief Subscribe notifications for a new SmartREST template.
     *
     *  Device push is able to listen notifications for multiple SmartREST
     *  templates. By adding another XID, the device push will also start
     *  listening notifications for this SmartREST template.
     *  \note Subscribe to same XID multiple times has no effect.
     *
     *  \param xid XID for the new SmartREST template.
     */
    void subscribe(const std::string &xid)
    {
        if (xids.find(xid) == std::string::npos)
        {
            xids += "," + xid;
            bayeuxState = (bayeuxState == BAYEUX_STATE_CONNECT) ? BAYEUX_STATE_SUBSCRIBE : bayeuxState;
            http.cancel();

            srInfo("push: subscribed to XID " + xid);
        }
    }

    /**
     *  \brief Unsubscribe notifications from a SmartREST template.
     *
     *  After unsubscribe, device push will no longer receive notifications
     *  for this SmartREST template.
     *  \note Unsubscribe from a previous not subscribed XID does nothing.
     *
     *  \param xid XID for the SmartREST template to be unsubscribed.
     */
    void unsubscribe(const std::string &xid)
    {
        const size_t pos = xids.find(xid);

        if (pos != std::string::npos)
        {
            xids.erase(pos - 1, xid.size() + 1);
            bayeuxState = (bayeuxState == BAYEUX_STATE_CONNECT) ? BAYEUX_STATE_SUBSCRIBE : bayeuxState;
            http.cancel();

            srInfo("push: unsubscribed from XID " + xid);
        }
    }

protected:
    /**
     *  \brief Implement the bayeux handshake process.
     */
    int handshake();

    /**
     *  \brief Implement the bayeux subscribe process.
     */
    int subscribe();

    /**
     *  \brief Implement the bayeux connect process.
     */
    int connect();

    /**
     *  \brief Process the received bayeux advice and the batch
     *  number for reliable push.
     *
     *  \note This function removes the bayeux advice and the batch number
     *  messages from s after processing.
     *
     *  \param s the entire response.
     */
    void process(std::string &s);

    /**
     *  \brief pthread routine.
     *
     *  \param arg a pointer to an SrDevicePush instance.
     */
    static void *func(void *arg);

private:

    enum BayeuxState
    {
        BAYEUX_STATE_IDLE = 0,
        BAYEUX_STATE_HANDSHAKE,
        BAYEUX_STATE_SUBSCRIBE,
        BAYEUX_STATE_CONNECT,
    };

    SrNetHttp http;
    pthread_t tid;
    std::string bayeuxID;
    std::string xids;
    size_t bnum;
    SrQueue<SrOpBatch> &queue;
    const std::string &channel;
    BayeuxState bayeuxState;
};

#endif /* SRDEVICEPUSH_H */
