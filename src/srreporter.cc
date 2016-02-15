#include <unistd.h>
#include <cstring>
#include "srreporter.h"
#ifndef SR_REPORTER_VAL
#define SR_REPORTER_VAL 400
#endif
#ifndef SR_REPORTER_NUM
#define SR_REPORTER_NUM 32
#endif
#ifndef SR_REPORTER_RETRIES
#define SR_REPORTER_RETRIES 10
#endif
using namespace std;


static void _insert(deque<string> &d, uint16_t cap, const string &s)
{
        if (d.size() >= cap) d.pop_front();
        d.emplace_back(s);
}


int SrReporter::start()
{
        int no = pthread_create(&tid, NULL, func, this);
        if (no) {
                srError("reporter: start failed, " + string(strerror(no)));
                return no;
        }
        srInfo("reporter: started.");
        return no;
}


void* SrReporter::func(void *arg)
{
        SrReporter *rep = (SrReporter*)arg;
        rep->http.setTimeout(20);
        string s;
        const string selector = "87,," + rep->xid + "\n";
        vector<string> vec;
        while (true) {
                s.clear();
                SrQueue<SrNews>::Event e = rep->out.get();
                if (e.second) continue;

                if (!rep->sleeping) {
                        for (auto &item: rep->buffer) s += item + "\n";
                }
                s += e.first.data + "\n";
                if (e.first.prio & 1)
                        _insert(rep->buffer, rep->_cap, e.first.data);

                // request aggregation
                for (int j = 1; j < SR_REPORTER_NUM; ++j) {
                        e = rep->out.get(SR_REPORTER_VAL);
                        if (e.second) break;

                        s += e.first.data + "\n";
                        if (e.first.prio & 1)
                                _insert(rep->buffer, rep->_cap, e.first.data);
                }
                if (rep->sleeping)
                        continue;

                // exponential waiting
                for (uint32_t i = 0; i < SR_REPORTER_RETRIES; ++i) {
                        if (rep->http.post(s) >= 0) {
                                rep->buffer.clear();
                                const string &resp = rep->http.response();
                                if (!resp.empty()) {
                                        rep->in.put(SrOpBatch(selector + resp));
                                        rep->http.clear();
                                }
                                break;
                        }
                        ::sleep(1 << i);
                }
        }
        return NULL;
}
