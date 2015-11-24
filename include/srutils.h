#ifndef SRUTILS_H
#define SRUTILS_H
#include <string>

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

#endif /* SRUTILS_H */
