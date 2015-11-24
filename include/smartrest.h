#ifndef SMARTREST_H
#define SMARTREST_H
#include <vector>
#include <string>
/**
 *  \class SrLexer
 *  \brief Lexical scanner for SmartREST messages.
 *
 *  A SmartREST message consists of a comma-separated-values.
 *  \note The value can contain white spaces, and escaped commas, double
 *  quote, control characters, etc.
 */
class SrLexer
{
public:
        /**
         *  \enum SrTokType
         *  \brief Token data type used by SrLexer for lexical scanning.
         */
        enum SrTokType {SR_NONE = 0, SR_ERROR, SR_STRING,
                        SR_INT, SR_FLOAT, SR_EOB, SR_NEWLINE};

        /**
         *  \typedef SrToken
         *  \brief Data structure used for holding a SmartREST token
         *  with its data type.
         */
        typedef std::pair<SrTokType, std::string> SrToken;
        /**
         *  \brief SrLexer constructor.
         *  \param _s the string for lexical scanning.
         */
        SrLexer(const std::string& _s): s(_s), i(0), delimit(false) {}
        virtual ~SrLexer() {}
        /**
         *  \brief Get the next token from the lexer.
         *
         *  This function is state-ful, no re-entrant. It scans over one CSV
         *  token and stops. In case the token has escaping double quotes,
         *  all of them are removed. This function implements an iterator
         *  that is commonly found in high-level languages.
         *
         *  \return the next CSV value with its type information as a token.
         */
        SrToken next();
        /**
         *  \brief Check if the given tokens is a delimiter for a
         *  SmartREST record.
         *
         *  A SmartREST record is a complete response, which consists of
         *  multiple CSV values. By definition, a SmartREST record ends with
         *  a non-escaped newline or end of buffer.
         *
         *  \param tok a CSV token to check
         *  \return true if the given token is a delimiter, false otherwise.
         */
        bool isdelimiter(const SrToken &tok) const {
                return tok.first == SR_NEWLINE || tok.first == SR_EOB;
        }
        /**
         *  \brief Reset the lexer with a new string.
         *
         *  This function clears all internal states of the lexer, and sets its
         *  buffer to the given string. The purpose of this function is for
         *  re-using an existing lexer, instead of create a new one every time.
         *
         *  \param _s the new string for lexing.
         */
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


/**
 *  \class SrRecord
 *  \brief Data structure represents a SmartREST record.
 *
 *  A SmartREST record is a list of CSVs (comma separated values), along with
 *  its scanned type returned from a SrLexer.
 */
class SrRecord
{
public:
        /**
         *  \brief SrRecord constructor.
         */
        SrRecord() {}
        virtual ~SrRecord() {}

        /**
         *  \brief Append a token to the record.
         *  \param tok token to be appended.
         */
        void push_back(SrLexer::SrToken &tok) {data.push_back(tok);}
        /**
         *  \brief Get the i-th token from the record.
         *  \param i index, cause undefined behavior if i is out of range.
         *  \return the token at position i.
         */
        const SrLexer::SrToken &operator[](size_t i) const {return data[i];}
        /**
         *  \brief Get the value of i-th token.
         *  \param i index, cause undefined behavior if i is out of range.
         *  \return the string representation at i-th token.
         */
        const std::string &value(size_t i) const {return data[i].second;}
        /**
         *  \brief Get the type of i-th token.
         *  \param i index, cause undefined behavior if i is out of range.
         *  \return the scanned type by a SrLexer.
         */
        SrLexer::SrTokType type(size_t i) const {return data[i].first;}
        /**
         *  \brief Get the type of i-th token at an integer.
         *
         *  This function is similar to the type function, except it returns
         *  an integer. The purpose of this function is for supporting
         *  binding with other languages where c++ enum is not supported.
         *
         *  \param i index, cause undefined behavior if i is out of range.
         *  \return the token type as an integer.
         */
        int typeInt(size_t i) const {return data[i].first;}
        /**
         *  \brief Return the size of the record.
         *  \return number of tokens in the record.
         */
        size_t size() const {return data.size();}

private:
        std::vector<SrLexer::SrToken> data;
};


/**
 *  \class SmartRest
 *  \brief SmartREST response parser.
 *
 *  As the SmartREST protocol supports message aggregation, multiple SmartREST
 *  messages can be aggregated into one response. This class is designed for
 *  parsing one single response, which in turn contains multiple SrRecord.
 */
class SmartRest
{
public:
        /**
         *  \brief SmartREST constructor. A container for a list of SrRecord.
         *  \param _s message contains the hold request or response.
         */
        SmartRest(const std::string &_s): lex(_s) {}
        virtual ~SmartRest() {}
        /**
         *  \brief Get the next SmartREST record.
         *
         *  This function is state-ful, no re-entrant. It parses the entire
         *  buffer with a SrLexer for one SrRecord and stops. This function
         *  implements an iterator that is commonly found in high-level
         *  languages.
         *
         *  \return the next SmartREST record.
         */
        SrRecord next() {
                SrRecord r;
                for (SrLexer::SrToken t = lex.next(); !lex.isdelimiter(t);
                     t = lex.next())
                        r.push_back(t);
                return r;
        }
        /**
         *  \brief Reset the SmartREST parser with a new buffer.
         *  \param _s reference to the new buffer.
         */
        void reset(const std::string &_s) { lex.reset(_s); }

private:
        SrLexer lex;
};

#endif /* SMARTREST_H */
