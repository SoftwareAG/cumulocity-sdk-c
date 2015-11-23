#ifndef SRBOOTSTRAP_H
#define SRBOOTSTRAP_H
#include <string>

/**
 *  \class SrBootstrap srbootstrap.h
 *  \brief Implementation of Cumulocity bootstrap process.
 *
 *  Bootstrap (or register) to Cumulocity is the process of requesting a device
 *  credential from the Cumulocity server or load the stored credential if the
 *  credential is already acquired earlier.
 */
class SrBootstrap
{
public:
        /**
         *  \brief SrBootstrap constructor.
         *  \param server Cumulocity server URL (no trailing slash).
         *  \param deviceID unique device ID (e.g., IMEI) for bootstrapping.
         */
        SrBootstrap(const std::string& server, const std::string &deviceID):
                _server(server+"/s"), _deviceID(deviceID) {}
        virtual ~SrBootstrap() {}

        /**
         *  \brief Get the Cumulocity tenant the device registered to.
         */
        const std::string& tenant() const {return _tenant;}
        /**
         *  \brief Get the received device username.
         */
        const std::string& username() const {return _username;}
        /**
         *  \brief Get the received device password.
         */
        const std::string& password() const {return _password;}
        /**
         *  \brief Reference implementation of the bootstrap process.
         *
         *  This function will be invoked by SrAgent to perform bootstrap during
         *  agent start-up. If you have special requirements, please subclass
         *  SrBootstrap and overwrite this function.
         *
         *  \param path path to store and load device credential.
         *  \return 0 on success, -1 on failure.
         */
        virtual int bootstrap(const std::string &path);

protected:
        /**
         *  \brief Load device credential. It will be called first by bootstrap.
         *  \param path path specifies the credential location.
         *  \return 0 on success, -1 on failure.
         */
        virtual int loadCredential(const std::string &path);
        /**
         *  \brief Request credential from Cumulocity. It will be called when
         *  loadCredential failed.
         *  \return 0 on success, -1 on failure.
         */
        virtual int requestCredential();
        /**
         *  \brief Save credential to storage when requestCredential succeeded.
         *
         *  Note this function will not create directories if they do not
         *  exist, you have to make sure all directories in the path exist
         *  and are write-able.
         *
         *  \param path path specifies the location for storing credentials.
         *  \return 0 on success, -1 on failure.
         */
        virtual int saveCredential(const std::string &path);

        const std::string _server;
        std::string _tenant;
        std::string _username;
        std::string _password;
        const std::string _deviceID;
};

#endif /* SRBOOTSTRAP_H */
