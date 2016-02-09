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
        virtual ~SrNetSocket() {}

        /**
         *  \brief Establish a new connection.
         *  \return 0 on success, -1 on failure.
         */
        int connect();
        /**
         *  \brief Socket send method.
         *  \note This function is for sending binary data that contains 0.
         *  \param buf pointer to the send buffer.
         *  \param len size of the send buffer.
         *  \return number of bytes sent (equal to len), -1 on failure.
         */
        int sendBuf(const char *buf, size_t len);
        /**
         *  \brief Socket send method.
         *  \param request request to be sent.
         *  \return number of bytes sent, (must equals to request size),
         *  -1 on failure.
         */
        int send(const string &request);
        /**
         *  \brief Socket recv method.
         *  \param len maximum bytes to receive (must not exceed
         *  SOCK_RECV_BUF_SIZE [default: 1024]).
         *  \return number of bytes received on success, -1 on failure.
         */
        int recv(size_t len);

private:
        const std::string _server;
};

#endif /* SRNETSOCKET_H */
