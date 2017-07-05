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

#ifndef SRNETHTTP_H
#define SRNETHTTP_H

#include <utility>
#include "srnetinterface.h"

/**
 *  \class SrNetHttp
 *  \brief Tailored HTTP implementation for Cumulocity SmartREST protocol.
 *  Supports both HTTP and HTTPS.
 */
class SrNetHttp: public SrNetInterface
{
public:
    /**
     *  \brief SrNetHttp constructor.
     *  \param server Cumulocity server URL (no trailing slash).
     *  \param xid registered SmartREST template ID from integration.
     *  \param auth authentication token get from SrAgent.
     */
    SrNetHttp(const std::string &server, const std::string &xid, const std::string &auth);
    virtual ~SrNetHttp();
    /**
     *  \brief HTTP post for SmartREST protocol.
     *
     *  This function is the workhorse for the SmartREST protocol. It is
     *  responsible for posting all requests, i.e., measurements, alarms,
     *  events, etc.
     *
     *  \param request one or multiple SmartREST requests
     *  \return size of response on success, -1 on failure.
     */
    int post(const std::string &request);
    /**
     *  \brief Cancel the current HTTP transaction.
     *
     *  This is an asynchronous function, ought to be called from another
     *  thread since the post() method will be blocking the current calling
     *  thread. This function will always succeed, but as its asynchronous
     *  feature, it only guarantees that the HTTP transaction is eventually
     *  canceled (normally less than a second). The post() method will then
     *  fail with errNo set to CURLE_ABORTED_BY_CALLBACK.
     *
     */
    void cancel()
    {
        meter.first = meter.second = 0;
    }

    /**
     *  \brief HTTP response status code. Undefined if post method failed.
     */
    int statusCode;

private:

    struct curl_slist *chunk;
    std::pair<time_t, time_t> meter;
};

#endif /* SRNETHTTP_H */
