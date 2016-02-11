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
 *  \note On success, the parameter *srv* will be updated with the registered
 *  SmartREST template XID, otherwise *srv* is untouched.
 *
 *  \param url Cumulocity server url, format <server>/s, where '/s' is endpoint.
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
 *  \param url Cumulocity server url, format <server>/s, where '/s' is endpoint.
 *  \param auth HTTP basic authorization token.
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
