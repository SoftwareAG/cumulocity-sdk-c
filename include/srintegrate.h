#ifndef SRINTEGRATE_H
#define SRINTEGRATE_H
#include <string>


class SrAgent;

class SrIntegrate
{
public:
        using string=std::string;
        SrIntegrate() {}
        virtual ~SrIntegrate() {}
        virtual int integrate(const SrAgent &agent, const string &srv,
                              const string &srt) = 0;
        const string &XID() const { return xid; }
        const string &ID() const { return id; }

protected:
        string xid;
        string id;
};

#endif /* SRINTEGRATE_H */
