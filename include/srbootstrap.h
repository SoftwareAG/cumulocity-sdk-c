#ifndef SRBOOTSTRAP_H
#define SRBOOTSTRAP_H
#include <string>


class SrBootstrap
{
public:
        SrBootstrap(const std::string& server, const std::string &deviceID):
                _server(server+"/s"), _deviceID(deviceID) {}
        virtual ~SrBootstrap() {}

        const std::string& tenant() const { return _tenant; }
        const std::string& username() const { return _username; }
        const std::string& password() const { return _password; }
        virtual int bootstrap(const std::string &path);

protected:
        virtual int loadCredential(const std::string &path);
        virtual int requestCredential();
        virtual int saveCredential(const std::string &path);

        std::string _server;
        std::string _tenant;
        std::string _username;
        std::string _password;
        const std::string _deviceID;
};

#endif /* SRBOOTSTRAP_H */
