#ifndef SRDEVICEPUSH_H
#define SRDEVICEPUSH_H
#include "srtypes.h"
#include "srqueue.h"
#include "srnetsocket.h"
#include "srlogger.h"


class SrDevicePush
{
public:
        /**
         *  \brief SrDevicePush constructor.
         *
         *  This class implements the real time notification in the SmartREST
         *  protocol. It maintains long polling connection to the server using
         *  the bayeux protocol (handshake, subscribe, connect), and block
         *  waiting for the response. To sustain the connection, a single space
         *  character as heartbeat is sent from the server every 10 minutes.
         *  When a batch of operation is received, it redo the connect or
         *  handshake based on the received bayeux advice. The class also
         *  features reliable push (the 88,<batch number> message as the end),
         *  to avoid missing operations due to a broken connection.
         *
         *  \param server the server url.
         *  \param xid eXternal ID of the registered SmartREST template.
         *  \param auth Authentication token get from the SrAgent.
         *  \param chn bayeux channel (the managed object ID of the device).
         *  \param queue reference to the ingress queue of the SrAgent.
         *  \return return type
         */
        SrDevicePush(const std::string &server, const std::string &xid,
                     const std::string &auth, const std::string &chn,
                     SrQueue<SrOpBatch> &queue);
        virtual ~SrDevicePush() {}

        /**
         *  \brief Start device push.
         *
         *  Device push requires a separate thread as it maintains a blocking
         *  connection to the server. This function creates a thread for device
         *  push and starts the real time notification.
         *
         *  \return 0 on success, non-0 if creating thread failed.
         */
        int start();
        /**
         *  \brief Check if the thread is sleeping.
         *
         *  \return true if sleeping, false otherwise.
         */
        bool isSleeping() const {return sleeping;}
        /**
         *  \brief Puts device push to sleep.
         *
         *  Notice the thread is still running, except the long polling
         *  connection is not maintained anymore, and all received operations
         *  are discarded. sleep an already sleeping device push has no effect.
         */
        void sleep() {
                sleeping = true;
                srNotice("push: slept.");
        }
        /**
         *  \brief Resume a formerly sleeping device push.
         *
         *  This function restarts the long polling connection again for
         *  receiving operations. resume a non-sleeping device push has no
         *  effect.
         */
        void resume() {
                sleeping = false;
                srNotice("push: resumed.");
        }

protected:
        /**
         *  \brief Implements the bayeux handshake process.
         */
        int handshake();
        /**
         *  \brief Implements the bayeux subscribe process.
         */
        int subscribe();
        /**
         *  \brief Implements the bayeux connect process.
         */
        int connect();
        /**
         *  \brief Process the received bayeux advice and the batch number for
         *  reliable push.
         *  \param s the entire response.
         */
        void process(std::string &s);
        /**
         *  \brief pthread routine.
         *
         *  \param arg a pointer to an SrDevicePush instance.
         */
        static void *func(void *arg);


private:
        SrNetSocket sock;
        pthread_t tid;
        SrQueue<SrOpBatch> &queue;
        std::string bayeuxID;
        const std::string channel;
        const std::string header;
        size_t bnum;
        uint8_t bayeuxPolicy;
        bool sleeping;
};

#endif /* SRDEVICEPUSH_H */
