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

#ifndef SRNETBINHTTP_H
#define SRNETBINHTTP_H

#include "srnetinterface.h"

/**
 *  \class SrNetBinHttp
 *  \brief Cumulocity HTTP binary API implementation.
 *
 *  Cumulocity binary APIs are used for uploading and downloading big files to
 *  and from your Cumulocity file repository.
 */
class SrNetBinHttp: public SrNetInterface
{
public:
    /**
     *  \brief SrNetBinHttp constructor.
     *  \param server Cumulocity server URL (no trailing space).
     *  \param auth authentication token from SrAgent.
     */
    SrNetBinHttp(const std::string &server, const std::string &auth);

    virtual ~SrNetBinHttp();
    /**
     *  \brief Cumulocity HTTP binary post.
     *  \param dest file name to be stored on the remote server.
     *  \param ct Content-Type of the actual data.
     *  \param data buffer contains the actual data.
     *  \return size of response on success, -1 on failure.
     */
    int post(const string &dest, const string &ct, const string &data);

    /**
     *  \brief Cumulocity HTTP binary post. Counterpart of post, except the
     *  data is read from a file.
     *  \param dest file name to be stored on the remote server.
     *  \param ct Content-Type of the actual data.
     *  \param file path to the file which stores the actual data.
     *  \return size of response on success, -1 on failure.
     */
    int postf(const string &dest, const string &ct, const string &file);

    /**
     *  \brief Cumulocity HTTP binary get.
     *  \param id Cumulocity binary resource unique identifier.
     *  \return size of response on success, -1 on failure.
     */
    int get(const string &id);

    /**
     *  \brief Cumulocity HTTP binary get.
     *
     *  Counterpart of the get API, except this function stores the response
     *  directly to a file. Suitable for large binary files.
     *  \note when using this function, the response can not be accessed by
     *  the resp function, rather stored in a file specified by argument path.
     *
     *  \param id Cumulocity binary resource unique identifier.
     *  \param dest local file path to store the response.
     *  \return size of file on success, -1 on failure.
     */
    int getf(const string &id, const string &dest);

private:

    string server;
    struct curl_slist *chunk;
};

#endif /* SRNETBINHTTP_H */
