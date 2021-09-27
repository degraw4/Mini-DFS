#ifndef _NAMESERVER_H
#define _NAMESERVER_H

#include <vector>
#include <map>
#include "dataserver.h"
#include "utils.h"
using namespace std;

class NameServer{
public:
    int fileNumber;
    int serverNumber;
    vector<DataServer *> dataServers;
    map<string, pair<int, int>> directory;

    NameServer(int number);
    vector<string> shell();
    void list();
    void operator()();
};

#endif