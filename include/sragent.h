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

#ifndef SRAGENT_H
#define SRAGENT_H

#include <map>
#include "smartrest.h"
#include "srqueue.h"
#include "srbootstrap.h"
#include "srintegrate.h"
#include "srtimer.h"

/**
 *  \class SrMsgHandler
 *  \brief Virtual abstract functor for SmartREST message callbacks.
 */
class SrMsgHandler
{
public:

    virtual ~SrMsgHandler()
    {
    }
    /**
     *  \brief SmartREST message handler interface.
     *  \param r the SmartREST message tokenized into an SrRecord.
     *  \param agent reference to the SrAgent instance.
     */
    virtual void operator()(SrRecord &r, SrAgent &agent) = 0;
};

typedef SrMsgHandler AbstractMsgHandler;

/**
 *  \class SrAgent
 *  \brief Main implementation of a device agent.
 *
 *  SrAgent implements a highly performant event-driven framework for scheduling
 *  all registered SrTimer instances and SmartREST message handlers. The agent
 *  controls the calling thread and runs silents in the background, until when
 *  a timeout event triggered by a SrTimer or a message with registered handler
 *  is received. The agent contains both an ingress and egress SrQueue, which
 *  are usually connected to an SrDevicePush and SrReporter for receiving
 *  responses and reporting requests.
 *  \note The agent does not guarantee accurate timer scheduling, which is
 *  especially true when the system is under heavy load. The only guarantee
 *  is that a timer will not be scheduled before its intended fire time.
 *  \note Do NOT rely on the agent to perform real-time scheduling.
 */
class SrAgent
{
private:
    using string = std::string;
public:
    typedef uint16_t MsgID;
    typedef uint32_t MsgXID;
    /**
     *  \brief SrAgent constructor.
     *
     *  \note It is highly discouraged to instantiate more than one SrAgent
     *  instance.
     *
     *  \param _server server URL (with no trailing slash).
     *  \param deviceid unique device ID for registration to Cumulocity.
     *  \param igt pointer to your own integration instance.
     *  \param boot pointer to your own bootstrap instance.
     */
    SrAgent(const string &_server, const string &deviceid, SrIntegrate *igt =
            NULL, SrBootstrap *boot = NULL);
    virtual ~SrAgent();

    /**
     *  \brief Get the Cumulocity tenant this device is registered to.
     */
    const string &tenant() const
    {
        return _tenant;
    }
    /**
     *  \brief Get the username this device received from registration.
     */
    const string &username() const
    {
        return _username;
    }
    /**
     *  \brief Get the password this device received from registration.
     */
    const string &password() const
    {
        return _password;
    }
    /**
     *  \brief Get the encode64 encoded username and password (for
     *  HTTP basic authorization).
     */
    const string &auth() const
    {
        return _auth;
    }
    /**
     *  \brief Get the server URL.
     */
    const string &server() const
    {
        return _server;
    }
    /**
     *  \brief Get the unique device ID.
     */
    const string &deviceID() const
    {
        return did;
    }
    /**
     *  \brief Get the eXternal ID of the registered SmartREST template
     *  from the integration process.
     */
    const string &XID() const
    {
        return xid;
    }
    /**
     *  \brief Get the managed object ID Cumulocity assigned to the device.
     */
    const string &ID() const
    {
        return id;
    }
    /**
     *  \brief Perform the registration process.
     *  \param path the path to store received credentials.
     *  \return 0 on success, -1 otherwise.
     */
    int bootstrap(const string &path);
    /**
     *  \brief Perform the integration process.
     *  \param srv SmartREST template version number
     *  \param srt SmartREST template content.
     *  \return 0 on success, -1 otherwise.
     */
    int integrate(const string &srv, const string &srt);
    /**
     *  \brief Put news to egress queue for reporting.
     *  \param news the news to report.
     *  \return 0 on success, -1 otherwise.
     */
    int send(const SrNews &news);
    /**
     *  \brief Enter the agent loop.
     *
     *  This function takes over the calling thread, starts the agent
     *  for scheduling.
     *  \note This function does not return.
     */
    void loop();
    /**
     *  \brief Add an SrTimer timer to the agent. Non thread-safe.
     *
     *  \note This function does not start the timer, you have to start
     *  the timer first.
     *  \note This function also does no existence check, adding the same
     *  timer multiple times results in the timer being added multiple times.
     *
     *  \param timer reference to an SrTimer to add to the agent.
     */
    void addTimer(SrTimer &timer)
    {
        timers.push_back(&timer);
    }
    /**
     *  \brief Add a message handler to the agent. Non thread-safe.
     *
     *  Register a new handler for the same message ID overwrites the old
     *  one. NULL clears the handler for this message.
     *
     *  \param msgid the message ID.
     *  \param functor pointer to a message handler.
     */
    void addMsgHandler(MsgID msgid, SrMsgHandler *functor)
    {
        handlers[msgid] = functor;
    }
    /**
     *  \brief Add a message handler to the agent. Non thread-safe.
     *
     *  Unlike addMsgHandler(), this function is for registering a message
     *  callback for additional SmartREST template that are added to
     *  SrDevicePush via subscribe() method.
     *
     *  Register a new handler for the same MsgXID and MsgID overwrites the
     *  old one. NULL clears the handler for this message.
     *
     *  \param msgxid XID of the SmartREST template.
     *  \param msgid the message ID
     *  \param f Pointer to an SrMsgHandler instance.
     */
    void addXMsgHandler(MsgXID msgxid, MsgID msgid, SrMsgHandler *f)
    {
        sh[XMsgID(msgxid, msgid)] = f;
    }

public:
    /**
     \brief Incoming queue for receiving SmartREST responses.
     */
    SrQueue<SrOpBatch> ingress;
    /**
     *  \brief Outgoing queue for sending SmartREST requests.
     */
    SrQueue<SrNews> egress;

private:
    void processMessages();

private:

    typedef std::map<MsgID, SrMsgHandler*> _Handler;
    typedef std::pair<MsgXID, MsgID> XMsgID;
    typedef std::map<XMsgID, SrMsgHandler*> _XHandler;
    typedef std::vector<SrTimer*> _Timer;
    typedef _Timer::iterator _TimerIter;
    _Timer timers;
    _Handler handlers;
    _XHandler sh;
    string _tenant;
    string _username;
    string _password;
    string _auth;
    const string _server;
    const string did;
    string xid;
    string id;
    SrBootstrap *pboot;
    SrIntegrate *pigt;
};

#endif /* SRAGENT_H */
