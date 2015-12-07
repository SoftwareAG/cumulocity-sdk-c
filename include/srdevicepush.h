#ifndef SRDEVICEPUSH_H
#define SRDEVICEPUSH_H
#include "srtypes.h"
#include "srqueue.h"
#include "srnethttp.h"
#include "srlogger.h"

/**
 * \class SrDevicePush
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
        /**
         *  \brief Subscribe notifications for a new SmartREST template.
         *
         *  Device push is able to listen notifications for multiple SmartREST
         *  templates. By adding another XID, the device push will also start
         *  listening notifications for this SmartREST template.
         *  \note Subscribe to same XID multiple times has no effect.
         *
         *  \param xid XID for the new SmartREST template.
         */
        void subscribe(const std::string &xid) {
                if (xids.find(xid) == std::string::npos) {
                        xids += "," + xid;
                        bayeuxPolicy = bayeuxPolicy == 3 ? 2 : bayeuxPolicy;
                }
        }
        /**
         *  \brief Unsubscribe notifications from a SmartREST template.
         *
         *  After unsubscribe, device push will no longer receive notifications
         *  for this SmartREST template.
         *  \note Unsubscribe from a previous not subscribed XID does nothing.
         *
         *  \param xid XID for the SmartREST template to be unsubscribed.
         */
        void unsubscribe(const std::string &xid) {
                const size_t pos = xids.find(xid);
                if (pos != std::string::npos) {
                        xids.erase(pos - 1, xid.size() + 1);
                        bayeuxPolicy = bayeuxPolicy == 3 ? 2 : bayeuxPolicy;
                }
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
        SrNetHttp http;
        pthread_t tid;
        std::string bayeuxID;
        std::string xids;
        size_t bnum;
        SrQueue<SrOpBatch> &queue;
        const std::string &channel;
        uint8_t bayeuxPolicy;
        bool sleeping;
};

#endif /* SRDEVICEPUSH_H */
