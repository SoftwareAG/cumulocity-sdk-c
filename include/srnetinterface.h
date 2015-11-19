#ifndef SRNETINTERFACE_H
#define SRNETINTERFACE_H
#include <string>
#include <curl/curl.h>


class SrNetInterface
{
public:
        typedef std::string string;
        SrNetInterface(const string &server);
        virtual ~SrNetInterface();

        void clear() {
                errNo = 0;
                *_errMsg = 0;
                resp.clear();
        }
        void setTimeout(long timeout);
        void setDebug(long l);

        const string& response() const {return resp;}
        const char *errMsg() const {return _errMsg;}
        int timeout() const {return t;}

        int errNo;
protected:
        CURL *curl;
        string resp;
        char _errMsg[CURL_ERROR_SIZE];

private:
        int t;
};

#endif /* SRNETINTERFACE_H */
