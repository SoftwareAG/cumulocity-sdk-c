#ifndef SRNETHTTP_H
#define SRNETHTTP_H
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
        SrNetHttp(const std::string &server, const std::string &xid,
                  const std::string &auth);
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
         *  \brief HTTP response status code. Undefined if post method failed.
         */
        int statusCode;
private:
        struct curl_slist *chunk;
};

#endif /* SRNETHTTP_H */
