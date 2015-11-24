#ifndef SRNETINTERFACE_H
#define SRNETINTERFACE_H
#include <string>
#include <curl/curl.h>

/**
 *  \class SrNetInterface
 *  \brief Base class of all networking classes.
 */
class SrNetInterface
{
protected:
        typedef std::string string;
public:
        /**
         *  \brief SrNetInterface constructor.
         *  \param server Cumulocity server URL (no trailing slash).
         */
        SrNetInterface(const string &server);
        virtual ~SrNetInterface();

        /**
         *  \brief Clear all internal states.
         *
         *  This function clears response buffer, errNo and errMsg buffer. Use
         *  this function when you want to reuse a connection.
         */
        void clear() {
                errNo = 0;
                *_errMsg = 0;
                resp.clear();
        }
        /**
         *  \brief Set timeout for the connection.
         *  \param timeout timeout in seconds.
         */
        void setTimeout(long timeout);
        /**
         *  \brief Enable/disable debug mode.
         *
         *  This function is intended for debugging purpose, when enabled, will
         *  print all internal libcurl networking transactions to stderr.
         *
         *  \param l 1 to enable, 0 to disable.
         */
        void setDebug(long l);
        /**
         *  \brief Get the response of last transaction.
         *
         *  \note The response is not cleared after each transaction, use the
         *  clear function if you do not want accumulated response.
         *  \return const reference to the response buffer.
         */
        const string& response() const {return resp;}
        /**
         *  \brief Get a human-readable error message.
         *
         *  \note You have to check the errNo first. If errNo is 0, then the
         *  content of the errMsg buffer is undefined.
         *
         *  \return const pointer to the errMsg buffer.
         */
        const char *errMsg() const {return _errMsg;}
        /**
         *  \brief Get the timeout parameter (in seconds).
         */
        int timeout() const {return t;}

        /**
         *  \brief errNo of last networking operation.
         */
        int errNo;
protected:
        /**
         *  \brief libcurl handle.
         */
        CURL *curl;
        /**
         *  \brief Response buffer.
         */
        string resp;
        /**
         *  \brief errMsg buffer.
         */
        char _errMsg[CURL_ERROR_SIZE];

private:
        int t;
};

#endif /* SRNETINTERFACE_H */
