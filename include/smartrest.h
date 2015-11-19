#ifndef SMARTREST_H
#define SMARTREST_H
#include <vector>
#include <string>


enum SrTokType {SR_NONE = 0, SR_ERROR, SR_STRING, SR_INT, SR_FLOAT,
                SR_EOB, SR_NEWLINE};
typedef std::pair<SrTokType, std::string> SrToken;


class SrLexer
{
public:
        SrLexer(const std::string& _s): s(_s), i(0), delimit(false) {}
        virtual ~SrLexer() {}
        SrToken next();
        bool isdelimiter(const SrToken &tok) const {
                return tok.first == SR_NEWLINE || tok.first == SR_EOB;
        }
        void reset(const std::string &_s) {
                s = _s;
                i = 0;
                delimit = false;
        }

private:
        std::string s;
        size_t i;
        bool delimit;
};


class SrRecord
{
public:
        SrRecord() {}
        virtual ~SrRecord() {}

        void push_back(SrToken &t) { data.push_back(t); }
        const SrToken &operator[](size_t i) const { return data[i]; }
        const std::string &value(size_t i) const { return data[i].second; }
        SrTokType type(size_t i) const { return data[i].first; }
        int typeInt(size_t i) const { return data[i].first; }
        size_t size() const { return data.size(); }

private:
        std::vector<SrToken> data;
};


class SmartRest
{
public:
        SmartRest(const std::string &_s): lex(_s) {}
        virtual ~SmartRest() {}
        SrRecord next() {
                SrRecord r;
                for (SrToken t = lex.next(); !lex.isdelimiter(t); t = lex.next())
                        r.push_back(t);
                return r;
        }
        void reset(const std::string &_s) { lex.reset(_s); }

private:
        SrLexer lex;
};

#endif /* SMARTREST_H */
