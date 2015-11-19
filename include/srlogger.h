#ifndef SRLOGGER_H
#define SRLOGGER_H
#include <string>

enum SrLogLevel{SRLOG_DEBUG = 0, SRLOG_INFO, SRLOG_NOTICE,
                SRLOG_WARNING, SRLOG_ERROR, SRLOG_CRITICAL};


/**
   Sets the logging destination.
   Remark: Not thread-safe. Must only be called once at the beginning of
   the program. Multiple invocation is undefined behavior.
   @filename destination file name.
 */
void srLogSetDest(const std::string &filename);
void srLogSetQuota(uint32_t quota);
uint32_t srLogGetQuota();
void srLogSetLevel(SrLogLevel lvl);
SrLogLevel srLogGetLevel();
bool srLogIsEnabledFor(SrLogLevel lvl);

void srDebug(const std::string &msg);
void srInfo(const std::string &msg);
void srNotice(const std::string &msg);
void srWarning(const std::string &msg);
void srError(const std::string &msg);
void srCritical(const std::string &msg);

#endif /* SRLOGGER_H */
