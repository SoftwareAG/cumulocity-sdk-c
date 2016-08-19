#include <algorithm>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include "srreporter.h"
#define Q_OK SrQueue<SrNews>::Q_OK
using namespace std;

#define SR_FILEBUF_VER 0x1
#define SR_FILEBUF_PAGE_BASE 9
#define SR_FILEBUF_PAGE_SIZE (1<<(SR_FILEBUF_PAGE_BASE+SR_FILEBUF_PAGE_SCALE))
#define SR_FILEBUF_INDEX_SUFFIX ".index"
#define SR_MEMBUF_SCALE 8
#define SR_MEMBUF_NUM (1 << SR_MEMBUF_SCALE)
#define BASE_PAGE(x) (x & 0x07)
#define BASE_VER(x) ((x >> 3) & 0x0f)
#define _BASE (BASE_PAGE(SR_FILEBUF_PAGE_SCALE) | (SR_FILEBUF_VER<<3))


struct _BFHead {
        _BFHead(): base(_BASE), flag(0), size(0), cnt(0), pad2(0) {}
        uint8_t base, flag;
        uint16_t size, cnt, pad2;
};


struct _BFPage {
        _BFPage(uint16_t idx = 0, uint16_t oft = 0, uint8_t f = 0):
                index(idx), offset(oft), flag(f), cnt(0), pad(0) {}
        uint16_t index, offset;
        uint8_t flag, cnt;
        uint16_t pad;
};

typedef std::deque<_BFPage> _PCB;
typedef std::vector<bool> _UFLAG;

static void readPCB(const string &fn, _BFHead &head, _PCB &pcb, _UFLAG &uflag)
{
        ifstream fs(fn, ios::binary);
        if (!fs.read((char*)&head, sizeof(head)))
                return;
        if (BASE_VER(head.base) != SR_FILEBUF_VER ||
            BASE_PAGE(head.base) != SR_FILEBUF_PAGE_SCALE) {
                head.cnt = head.size = 0;
                return;
        }
        const size_t sz = head.size;
        if (uflag.size() < sz)
                uflag.resize(sz);
        _BFPage page;
        uint8_t flag = 2;
        uint16_t cnt = 0;
        for (size_t i = 0; fs.read((char*)&page, sizeof(page)) && i < sz; ++i) {
                pcb.push_back(page);
                uflag[page.index] = true;
                cnt += flag == page.flag ? 0 : 1;
                flag = page.flag;
        }
        head.size = pcb.size();
        head.cnt = cnt;
}


static void writePCB(const string &fn, const _BFHead &head, _PCB &pcb)
{
        ofstream out(fn, ios::binary);
        if (!out.write((const char*)&head, sizeof(head)))
                return;
        for (const auto &e: pcb) {
                if (!out.write((const char*)&e, sizeof(_BFPage)))
                        break;
        }
}


static int readPage(ifstream &in, size_t i, char *dest, size_t size)
{
        in.seekg(i * SR_FILEBUF_PAGE_SIZE);
        return in.read(dest, size) ? in.gcount() : -1;
}


static int writePage(ofstream &out, size_t index, const char *buf,
                     size_t size, size_t offset = 0)
{
        out.seekp(index * SR_FILEBUF_PAGE_SIZE + offset);
        return out.write(buf, size) ? 0 : -1;
}


class _Pager
{
public:
        _Pager(uint16_t _cap): cap(_cap) {}
        virtual ~_Pager() {}

        size_t capacity() const {return cap;};
        void setCapacity(uint16_t _cap) {cap = _cap;};
        virtual bool empty() const = 0;
        virtual size_t bsize() const = 0;
        virtual size_t size() const = 0;
        virtual string front() const = 0;
        virtual void pop_front() = 0;
        virtual int emplace_back(const string &s) = 0;
        virtual void clear() = 0;

protected:
        uint16_t cap;
};


class _BFPager: public _Pager {
public:
        _BFPager(const string &_fn, int c): _Pager(c), fn(_fn), uflag(c) {
                readPCB(fn + SR_FILEBUF_INDEX_SUFFIX, head, pcb, uflag);
                if (access(fn.c_str(), F_OK) == -1) ofstream out(fn);
        }
        ~_BFPager() {writePCB(fn + SR_FILEBUF_INDEX_SUFFIX, head, pcb);}

        virtual bool empty() const {return head.size == 0;}
        virtual size_t bsize() const {return head.cnt;}
        virtual size_t size() const {return head.size;}
        virtual string front() const {
                string s;
                char buf[SR_FILEBUF_PAGE_SIZE];
                ifstream in(fn, ios::binary);
                const auto flag = pcb.front().flag;
                for (size_t i = 0; i < pcb.size() && flag == pcb[i].flag; ++i) {
                        const auto offset = pcb[i].offset + 1;
                        if (readPage(in, pcb[i].index, buf, offset) != offset)
                                break;
                        s.append(buf, offset);
                }
                return s;
        };
        virtual void pop_front() {
                const auto flag = pcb.front().flag;
                size_t i = 0;
                for (; i < pcb.size() && pcb[i].flag == flag; ++i)
                        uflag[pcb[i].index] = false;
                pcb.erase(pcb.begin(), pcb.begin() + i);
                head.size = pcb.size();
                --head.cnt;
                writePCB(fn + SR_FILEBUF_INDEX_SUFFIX, head, pcb);
        }
        virtual int emplace_back(const string &s) {
                const auto sz = SR_FILEBUF_PAGE_SIZE;
                if (pcb.empty() || s.size() + pcb.back().offset > sz) {
                        return push_back(s);
                } else {
                        auto &t = pcb.back();
                        const auto f = t.offset + 1;
                        ofstream out(fn, ios::binary | ios::in);
                        if (writePage(out, t.index, s.c_str(), s.size(), f))
                                return -1;
                        t.offset += s.size();
                        writePCB(fn + SR_FILEBUF_INDEX_SUFFIX, head, pcb);
                        return 0;
                }
        }
        virtual void clear() {
                pcb.clear();
                fill(uflag.begin(), uflag.end(), false);
                head.size = head.cnt = 0;
                writePCB(fn + SR_FILEBUF_INDEX_SUFFIX, head, pcb);
                const auto c = cap;
                if (c < uflag.size()) {
                        truncate(fn.c_str(), c * SR_FILEBUF_PAGE_SIZE);
                        uflag.resize(c);
                        srInfo("filebuf: truncate " + to_string(c));
                }
        }

private:
        virtual int push_back(const string &s) {
                const auto _cap = cap;
                if (uflag.size() < _cap) uflag.resize(_cap);

                const uint8_t flag = (pcb.empty() || pcb.back().flag) ? 0 : 1;
                const auto sz = SR_FILEBUF_PAGE_SIZE;
                const int N = s.size() / sz;
                ofstream out(fn, ios::binary | ios::in);
                const char *buf = s.c_str();
                for (int i = 0; i < N; ++i) {
                        const auto index = get_free_page();
                        if (writePage(out, index, buf + i * sz, sz) == -1)
                                return -1;
                        uflag[index] = true;
                        pcb.emplace_back(index, sz - 1, flag);
                }
                const auto c = s.size() & (sz - 1);
                if (c) {
                        const auto index = get_free_page();
                        if (writePage(out, index, buf + N * sz, c) == 0) {
                                uflag[index] = true;
                                pcb.emplace_back(index, c - 1, flag);
                        }
                }
                head.size = pcb.size();
                ++head.cnt;
                writePCB(fn + SR_FILEBUF_INDEX_SUFFIX, head, pcb);
                return 0;
        }
        uint16_t get_free_page() {
                uint16_t index = 0;
                for (; index < cap && uflag[index]; ++index);
                if (index >= cap) {
                        index = pcb.front().index;
                        pop_front();
                }
                return index;
        }

        std::deque<_BFPage> pcb;
        std::string fn;
        _BFHead head;
        std::vector<bool> uflag;
};


class _MemPager: public _Pager
{
public:
        _MemPager(uint16_t _cap): _Pager(_cap) {}
        virtual ~_MemPager() {}

        virtual bool empty() const {return mcb.empty();}
        virtual size_t bsize() const {
                const auto s = mcb.size();
                const auto b = s & (SR_MEMBUF_NUM - 1);
                return (s >> SR_MEMBUF_SCALE) + (b ? 1 : 0);
        }
        virtual size_t size() const {return mcb.size();}
        virtual string front() const {
                string s;
                for (size_t i = 0; i < mcb.size() && i < SR_MEMBUF_NUM; ++i)
                        s += mcb[i];
                return s;
        }
        virtual void pop_front() {
                auto p = [](const string &T) {return !T.compare(0, 3, "15,");};
                if (mcb.size() <= SR_MEMBUF_NUM) {
                        mcb.clear();
                } else if (p(mcb[SR_MEMBUF_NUM])) {
                        mcb.erase(mcb.begin(), mcb.begin() + SR_MEMBUF_NUM);
                } else {
                        const auto a = mcb.size() - SR_MEMBUF_NUM;
                        auto it = find_if(mcb.rbegin() + a, mcb.rend(), p);
                        const auto s = *it;
                        mcb.erase(mcb.begin(), mcb.begin() + SR_MEMBUF_NUM);
                        mcb.push_front(s);
                }
        }
        virtual int emplace_back(const string &s) {
                auto p = [](const string &T) {return T.compare(0, 3, "15,");};
                if (mcb.size() >= cap) {
                        if (p(mcb.front())) {
                                mcb.pop_front();
                        } else {
                                const string front(mcb.front());
                                mcb.erase(mcb.begin(), mcb.begin() + 2);
                                if (!mcb.empty() && p(mcb.front()))
                                        mcb.emplace_front(front);
                        }
                }
                mcb.emplace_back(s);
                return 0;
        }
        virtual void clear() {mcb.clear();}

private:
        deque<string> mcb;
};


SrReporter::SrReporter(const string &s, const string &x, const string &a,
                       SrQueue<SrNews> &out, SrQueue<SrOpBatch> &in,
                       uint16_t cap, const string fn):
        http(s + "/s", "", a), out(out), in(in), xid(x),
        ptr(), sleeping(false), filebuf(!fn.empty())
{
        if (filebuf)
                ptr.reset(new _BFPager(fn, cap));
        else
                ptr.reset(new _MemPager(cap));
}


SrReporter::~SrReporter() {}
uint16_t SrReporter::capacity() const {return ptr->capacity();}
void SrReporter::setCapacity(uint16_t cap) {ptr->setCapacity(cap);}


int SrReporter::start()
{
        pthread_t tid;
        int no = pthread_create(&tid, NULL, func, this);
        if (no)
                srError("reporter: start failed, " + string(strerror(no)));
        return no;
}


static string aggregate(SrQueue<SrNews> *q, _Pager *p, bool bf,
                        const string &xid, bool k)
{
        string s, buf;
        string myxid;
        for (int j = 0; j < SR_REPORTER_NUM; ++j) {
                auto e = q->get(SR_REPORTER_VAL);
                if (e.second != Q_OK) break;
                const string &data = e.first.data;
                string cxid = xid;
                size_t pos = 0;
                if (e.first.prio & SR_PRIO_XID) { // alternate XID
                        pos = data.find(',');
                        cxid = data.substr(0, pos++);
                }
                if (cxid != myxid) {
                        myxid = cxid;
                        if (k)
                                s += "15," + myxid + '\n';
                        if (e.first.prio & SR_PRIO_BUF) {
                                if (bf) buf += "15," + myxid + '\n';
                                else p->emplace_back("15," + myxid + '\n');
                        }
                }
                if (k)
                        s += data.substr(pos) + '\n';
                if (e.first.prio & SR_PRIO_BUF) {
                        if (bf) buf += data.substr(pos) + '\n';
                        else p->emplace_back(data.substr(pos) + '\n');
                }
        }
        if (bf && !buf.empty())
                p->emplace_back(buf);
        return s;
}


static void insert_resp(SrQueue<SrOpBatch> &in, SrNetHttp &http)
{
        const string &resp = http.response();
        if (!resp.empty()) in.put(SrOpBatch(resp));
        http.clear();
}


void *SrReporter::func(void *arg)
{
        SrReporter *rpt = (SrReporter*)arg;
        auto p = rpt->ptr.get();
        const bool bf = rpt->filebuf;
        rpt->http.setTimeout(30);
        if (bf) {
                const string s = "filebuf: " + to_string(SR_FILEBUF_VER);
                srNotice(s + ", " + to_string(SR_FILEBUF_PAGE_SIZE));
        }
        srInfo("reporter: buf capacity: " + to_string(p->capacity()));
        if (!rpt->sleeping) {
                bool empty = p->empty();
                string s = aggregate(&rpt->out, p, bf, rpt->xid, true);
                if (!s.empty()) {
                        for (uint32_t i = 0; i < SR_REPORTER_RETRIES; ++i) {
                                if (rpt->http.post(s) >= 0) {
                                        if (empty) p->clear();
                                        break;
                                }
                                ::sleep(1 << i);
                        }
                        insert_resp(rpt->in, rpt->http);
                }
        }
        srInfo("reporter: listening...");
        while (true) {
                // pre-fetching
                string s = p->front();
                bool k = (p->bsize() <= 1);
                s += aggregate(&rpt->out, p, bf, rpt->xid, k);
                // sleeping mode
                if (rpt->sleeping || s.empty()) continue;
                // exponential wait
                for (uint32_t i = 0; i < SR_REPORTER_RETRIES; ++i) {
                        if (rpt->http.post(s) >= 0) {
                                if (k) p->clear();
                                else if (!p->empty()) p->pop_front();
                                break;
                        }
                        ::sleep(1 << i);
                }
                insert_resp(rpt->in, rpt->http);
        }
        return NULL;
}
