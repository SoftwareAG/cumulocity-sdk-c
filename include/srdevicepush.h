#ifndef SRDEVICEPUSH_H
#define SRDEVICEPUSH_H
#include "srtypes.h"
#include "srqueue.h"
#include "srnetsocket.h"
#include "srlogger.h"

/**
 * \class SrDevicePush srdevicepush.h
 * \brief SmartREST real-time notification implementation.
 *
 *  This class implements the real-time notification in the SmartREST
 *  protocol. It maintains long polling connection to the server using
 *  the bayeux protocol (handshake, subscribe, connect), and block waiting
 *  for the response. To sustain the connection, a single space character
 *  as heartbeat is sent from the server every 10 minutes. When a batch of
 *  operation is received, it redo the connect or handshake based on the
 *  received bayeux advice. The class also features reliable push
 *  (message: 88,batch number) to avoid missing operations due to a
 *  broken connection.
 */
class SrDevicePush
{
public:
        /**
         *  \brief SrDevicePush constructor.
         *  \param server the server URL.
         *  \param xid eXternal ID of the registered SmartREST template.
         *  \param auth Authentication token get from the SrAgent.
         *  \param chn bayeux channel (the managed object ID of the device).
         *  \param queue reference to the ingress queue of the SrAgent.
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
         *  \brief Check if device push is sleeping.
         *
         *  \return true if sleeping, false otherwise.
         */
        bool isSleeping() const {return sleeping;}
        /**
         *  \brief Put device push to sleep.
         *
         *  \note The thread is still running, except the long polling
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
         *  \brief Implement the bayeux handshake process.
         */
        int handshake();
        /**
         *  \brief Implement the bayeux subscribe process.
         */
        int subscribe();
        /**
         *  \brief Implement the bayeux connect process.
         */
        int connect();
        /**
         *  \brief Process the received bayeux advice and the batch
         *  number for reliable push.
         *
         *  \note This function removes the bayeux advice and the batch number
         *  messages from s after processing.
         *
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
