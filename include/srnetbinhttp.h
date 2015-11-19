#ifndef SRNETBINHTTP_H
#define SRNETBINHTTP_H
#include "srnetinterface.h"


class SrNetBinHttp: public SrNetInterface
{
public:
        SrNetBinHttp(const std::string &server, const std::string &auth);
        virtual ~SrNetBinHttp();
        int post(const string &dest, const string &ct, const string &data);
        int postf(const string &dest, const string &ct, const string &file);
        int get(const string &id);
        int getf(const string &id, const string &dest);
private:
        string server;
        struct curl_slist *chunk;
};

#endif /* SRNETBINHTTP_H */
