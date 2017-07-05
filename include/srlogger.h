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

#ifndef SRLOGGER_H
#define SRLOGGER_H

#include <cstdint>
#include <string>

/**
 *  \file srlogger.h
 *  \brief SmartREST logging facility with rotation feature.
 */
enum SrLogLevel{SRLOG_DEBUG = 0, SRLOG_INFO, SRLOG_NOTICE,
                SRLOG_WARNING, SRLOG_ERROR, SRLOG_CRITICAL};


/**
 *  \brief Set logging destination. Default to terminal if not set.
 *
 *  \note This function should at maximum be called once at the beginning of the
 *  program. (Re-)set logging destination at the middle of the program or from
 *  other main threads cause undefined behavior.
 *
 *  \note You have to make sure the logging destination is write-able, otherwise
 *  this function has no effect.
 *
 *  \param filename destination file name.
 */
void srLogSetDest(const std::string &filename);
/**
 *  \brief Set logging file quota.
 *
 *  The quota is only used when logging destination is not terminal. When
 *  the quota is exceeded, the internal logger does log rotation and keeps
 *  at maximum 2 back-ups.
 *
 *  \note Due to I/O buffering, the size of log file may be slightly bigger
 *  than the quota.
 *
 *  \param quota maximum log file size in KB.
 */
void srLogSetQuota(uint32_t quota);
/**
 *  \brief Get log quota in KB.
 */
uint32_t srLogGetQuota();
/**
 *  \brief Set minimum logging level. Logs with lower logging level will
 *  be discarded.
 */
void srLogSetLevel(SrLogLevel lvl);
/**
 *  \brief Get current minimum logging level.
 */
SrLogLevel srLogGetLevel();
/**
 *  \brief Check if logging is enabled for level lvl.
 *
 *  This function is mainly usable when constructing the logging message is
 *  time or resource consuming.
 *
 *  For example, when use *srDebug(foo() + bar())* and foo() and bar() take
 *  time and CPU cycle to execute and the message is discarded when debug
 *  mode is not enabled.
 *
 *  \param lvl logging level for checking.
 *  \return true if logging is enabled for level lvl, false otherwise.
 */
bool srLogIsEnabledFor(SrLogLevel lvl);
/**
 *  \brief Log a message in DEBUG level.
 */
void srDebug(const std::string &msg);
/**
 *  \brief Log a message in INFO level.
 */
void srInfo(const std::string &msg);
/**
 *  \brief Log a message in NOTICE level.
 */
void srNotice(const std::string &msg);
/**
 *  \brief Log a message in WARNING level.
 */
void srWarning(const std::string &msg);
/**
 *  \brief Log a message in ERROR level.
 */
void srError(const std::string &msg);
/**
 *  \brief Log a message in CRITICAL level.
 */
void srCritical(const std::string &msg);

#endif /* SRLOGGER_H */
