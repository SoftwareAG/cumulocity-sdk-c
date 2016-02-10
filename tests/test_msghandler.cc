#include <iostream>
#include <cstdlib>
#include <cassert>
#include <sragent.h>
using namespace std;


class Callback: public AbstractMsgHandler
{
public:
        Callback(const vector<string> &vec): vec(vec) {}
        void operator()(SrRecord &r, SrAgent &agent) {
                std::vector<string> res;
                for (int i = 0; i < r.size(); ++i) res.push_back(r[i].second);
                assert(vec == res);
                cerr << "OK!" << endl;
                exit(0);
        }
        virtual ~Callback() {}
private:
        const vector<string> vec;
};



int main()
{
        cerr << "Test MSG Handler: ";
        SrAgent agent("", "", NULL, NULL);
        vector<string> vec {"151", "329", "payload"};
        SrOpBatch op;
        for (auto &e: vec) op.data += e + ",";
        op.data.erase(op.data.size() - 1);
        Callback callback(vec);
        agent.addMsgHandler(151, &callback);
        agent.ingress.put(op);
        agent.loop();
        return 0;
}
