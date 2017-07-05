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

#ifndef SRTYPES_H
#define SRTYPES_H

#include <string>

#define SR_PRIO_BUF 1
#define SR_PRIO_XID 2

/**
 *  \class SrNews
 *  \brief Data type represents a SmartREST request (measurement, alarm, etc.).
 */
struct SrNews
{
    /**
     *  \brief SrNews default constructor.
     *
     *  Construct a SrNews with empty string and prio \a prio.
     *
     *  \param prio assignment to member prio.
     */
    SrNews(uint8_t prio = 0) :
            prio(prio)
    {
    }

    /**
     *  \brief SrNews constructor.
     *
     *  Construct a SrNews with string \a s and prio \a prio.
     *
     *  \param s string reference for assigning member \a data.
     *  \param prio assignment to member \a prio.
     */
    SrNews(const std::string &s, uint8_t prio = 0) :
            data(s), prio(prio)
    {
    }

    /**
     *  \brief The request to send to Cumulocity.
     */
    std::string data;

    /**
     *  \brief request priority.
     *
     *  \a 0: default.
     *
     *  \a SR_PRIO_BUF: request will be buffered if send fails, and will be
     *  re-tried latter. Buffering still does not mean 100% guarantee,
     *  as the buffer of the SrReporter has an explicit capacity.
     *  Old requests will be discarded if the capacity is exhausted.
     *
     *  \a SR_PRIO_XID: request uses a different template XID than
     *  SrAgent.XID(), and the first field in the CSV is the alternate XID.
     *
     *  \note prio can be bit-wise XOR-ed, multiple priority can be set
     *  at the same time.
     */
    uint8_t prio;
};

/**
 *  \class SrOpBatch
 *  \brief Data type represents a SmartREST response, i.e., a batch of
 *  multiple messages.
 */
struct SrOpBatch
{
    /**
     *  \brief SrOpBatch default constructor.
     */
    SrOpBatch()
    {
    }
	
    /**
     *  \brief SrOpBatch constructor.
     *
     *  Construct a SrOpBatch with string \a s.
     *
     *  \param s string
     */
    SrOpBatch(const std::string &s) :
            data(s)
    {
    }
	
    /**
     *  \brief Buffer contains the response.
     */
    std::string data;
};

#endif /* SRTYPES_H */
