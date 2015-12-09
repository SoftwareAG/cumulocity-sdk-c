#include <unistd.h>
#include <cstring>
#include "srreporter.h"
using namespace std;


static void _insert(deque<string> &d, uint16_t cap, string &buf,
                    const SrQueue<SrNews>::Event &e)
{
        if (e.first.prio & 1) {
                if (d.size() >= cap)
                        d.pop_front();
                d.push_back(e.first.data + "\n");
        } else {
                buf += e.first.data + "\n";
        }
}


static string _pack(const deque<string> &d, const string &buf)
{
        string s;
        typedef deque<string>::const_iterator myiter;
        for (myiter iter = d.begin(); iter != d.end(); ++iter)
                s += *iter;
        return s + buf;
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
        string aggre;
        while (true) {
                aggre.clear();
                SrQueue<SrNews>::Event e = rep->out.get();
                if (e.second != SrQueue<SrNews>::Q_OK)
                        continue;
                _insert(rep->buffer, rep->_cap, aggre, e);

                e = rep->out.get(400);
                for (uint8_t i = 0; e.second == SrQueue<SrNews>::Q_OK && i < 40;
                     ++i, e = rep->out.get(400))
                        _insert(rep->buffer, rep->_cap, aggre, e);
                if (aggre.empty() || rep->sleeping)
                        continue;

                const string s = _pack(rep->buffer, aggre);
                for (uint16_t i = 2; i & 1023; i <<= 1) {
                        if (rep->http.post(s) >= 0) {
                                rep->buffer.clear();
                                const string &resp = rep->http.response();
                                if (!resp.empty()) {
                                        rep->in.put(SrOpBatch(resp));
                                        rep->http.clear();
                                }
                                break;
                        }
                        ::sleep(i);
                }
        }
        return NULL;
}
