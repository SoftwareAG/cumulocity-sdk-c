#ifndef SRNETSOCKET_H
#define SRNETSOCKET_H
#include "srnetinterface.h"


class SrNetSocket: public SrNetInterface
{
public:
        SrNetSocket(const string &server);
        virtual ~SrNetSocket() {}

        int connect();
        int send(const string &request);
        int recv(size_t len);

private:
        long sockextr;
        curl_socket_t sockfd;
};

#endif /* SRNETSOCKET_H */
