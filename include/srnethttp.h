#ifndef SRNETHTTP_H
#define SRNETHTTP_H
#include "srnetinterface.h"


class SrNetHttp: public SrNetInterface
{
public:
        SrNetHttp(const std::string &server, const std::string &xid,
                  const std::string &auth);
        virtual ~SrNetHttp();
        int post(const std::string &request);

        int statusCode;
private:
        struct curl_slist *chunk;
};

#endif /* SRNETHTTP_H */
