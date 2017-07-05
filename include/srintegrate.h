/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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

    virtual ~SrIntegrate()
    {
    }
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
    const string &XID() const
    {
        return xid;
    }
    /**
     *  \brief Get the managed object ID for the integrated device.
     */
    const string &ID() const
    {
        return id;
    }

protected:

    string xid;
    string id;
};

#endif /* SRINTEGRATE_H */
