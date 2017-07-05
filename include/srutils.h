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

#ifndef SRUTILS_H
#define SRUTILS_H

#include <string>
#include "srnethttp.h"

/**
 *   \file srutils.h
 *   \brief Misc SmartREST utility functions.
 */

/**
 *  \brief Read a SmartREST template from file.
 *
 *  The required file format is: first line contains and only contains the
 *  template version. After then, every line contains a complete SmartREST
 *  request or response template. Every empty line or line starts with a '#'
 *  sign (no leading spaces) is omitted.
 *  \note You must ensure there is no trailing spaces at each line, otherwise
 *  the template registration will fail.
 *
 *  \param path path specifies the file contains the SmartREST template.
 *  \param srv the returned SmartREST template version number.
 *  \param srt the returned SmartREST template content.
 *  \return 0 on success, -1 otherwise.
 */
int readSrTemplate(const std::string &path, std::string &srv, std::string &srt);
/**
 *  \brief Register a SmartREST template.
 *
 *  This function checks if the SmartREST template already exists in the server
 *  and then register it if it doesn't exist yet.
 *  \note On success, the parameter \a srv will be updated with the registered
 *  SmartREST template XID, otherwise \a srv is untouched.
 *
 *  \param url Cumulocity server url, format \a server/s, where \a /s is endpoint.
 *  \param auth HTTP basic authorization token.
 *  \param srv SmartREST version number.
 *  \param srt SmartREST template content.
 *  \return 0 on success, -1 otherwise.
 */
int registerSrTemplate(const std::string &url, const std::string &auth,
                       std::string &srv, const std::string &srt);
/**
 *  \brief Register a SmartREST template.
 *
 *  This function encourages re-using an existing HTTP connection instead of
 *  creating a new HTTP connection every time, it's preferred over its counterpart
 *  since the former one creates a new connection every time.
 *
 *  \param http reference to an existing SrNetHttp instance.
 *  \param srv will be written with SmartREST template XID on success.
 *  \param srt SmartREST template content.
 *  \return 0 on success, -1 otherwise.
 */
int registerSrTemplate(SrNetHttp &http, std::string &srv, const std::string &srt);

/**
 *  \brief Base64 encode (for HTTP basic authorization).
 *  \param s string for base64 encoding.
 *  \return Encoded string.
 */
std::string b64Encode(const std::string &s);
/**
 *  \brief Base64 decode (for HTTP basic authorization)
 *  \param s string for base64 decoding.
 *  \return Decoded string.
 */
std::string b64Decode(const std::string &s);

#endif /* SRUTILS_H */
