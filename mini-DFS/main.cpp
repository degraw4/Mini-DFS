#include <iostream>
#include <thread>
#include <vector>
#include "nameserver.h"
#include "dataserver.h"
using namespace std;

int main(){
    NameServer nameServer(4);
    DataServer dataServer_1("DataServer_1");
    DataServer dataServer_2("DataServer_2");
    DataServer dataServer_3("DataServer_3");
    DataServer dataServer_4("DataServer_4");

    nameServer.dataServers.push_back(&dataServer_1);
    nameServer.dataServers.push_back(&dataServer_2);
    nameServer.dataServers.push_back(&dataServer_3);
    nameServer.dataServers.push_back(&dataServer_4);

    thread thread_1(ref(dataServer_1));
    thread thread_2(ref(dataServer_2));
    thread thread_3(ref(dataServer_3));
    thread thread_4(ref(dataServer_4));

    thread_1.detach();
    thread_2.detach();
    thread_3.detach();
    thread_4.detach();

    nameServer();

    return 0;
}
