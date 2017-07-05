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

#ifndef SRNETSOCKET_H
#define SRNETSOCKET_H

#include "srnetinterface.h"

/**
 *  \class SrNetSocket
 *  \brief Low-level socket interface implementation.
 *
 *  SrNetSocket implements low-level socket interface with the advantage of
 *  automatically setting up the connection for you. Specifically, SrNetSocket
 *  supports TLS layer, it will set up the connection along with TLS handshake
 *  to ease your development.
 */
class SrNetSocket: public SrNetInterface
{
public:
    /**
     *  \brief SrNetSocket constructor.
     *  \param server Cumulocity server URL.
     */
    SrNetSocket(const string &server);
    virtual ~SrNetSocket()
    {
    }

    /**
     *  \brief Establish a new connection.
     *  \return 0 on success, -1 on failure.
     */
    int connect();
    /**
     *  \brief Socket send method.
     *  \param buf pointer to the send buffer.
     *  \param len size of the send buffer.
     *  \return number of bytes sent, which might be smaller than \a len
     *  or even 0, -1 on failure.
     */
    int sendBuf(const char *buf, size_t len);
    /**
     *  \brief Socket send method.
     *  \param request request to be sent.
     *  \return number of bytes sent, which might be smaller than
     *  \a request.size() or even 0, -1 on failure.
     */
    int send(const string &request);
    /**
     *  \brief Socket recv method.
     *  \param len maximum bytes to receive (must not exceed
     *  SOCK_RECV_BUF_SIZE [default: 1024]).
     *  \return number of bytes received on success, -1 on failure.
     *
     *  \note check errNo == CURLE_AGAIN if -1 is returned. In this case,
     *  it simply indicates you should call \a recv() again, not necessarily
     *  signals a network error.
     */
    int recv(size_t len);

private:

    const std::string _server;
};

#endif /* SRNETSOCKET_H */
