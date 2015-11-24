#ifndef SRINTEGRATE_H
#define SRINTEGRATE_H
#include <string>


class SrAgent;

/**
 *  \class SrIntegrate
 *  \brief Interface class for defining your own integration process.
 *
 *  This function defines the interface API to be called by SrAgent after
 *  bootstrap. You should subclass this virtual class to implement your own
 *  integrate process to Cumulocity.
 */
class SrIntegrate
{
protected:
        using string=std::string;
public:
        virtual ~SrIntegrate() {}
        /**
         *  \brief Overwrite this function to implement your integration process.
         *  \param agent reference to the SrAgent instance.
         *  \param srv SmartREST template version.
         *  \param srt SmartREST template content.
         *  \return 0 on success, -1 on failure.
         */
        virtual int integrate(const SrAgent &agent, const string &srv,
                              const string &srt) = 0;
        /**
         *  \brief Return the eXternal ID for the registered SmartREST template.
         */
        const string &XID() const {return xid;}
        /**
         *  \brief Get the managed object ID for the integrated device.
         */
        const string &ID() const {return id;}

protected:
        string xid;
        string id;
};

#endif /* SRINTEGRATE_H */
