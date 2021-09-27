#ifndef _DATASERVER_H
#define _DATASERVER_H

#include <string>
#include <mutex>
#include <condition_variable>
using namespace std;

class DataServer{
public:
    string serverName;
    int fileID;
    int chunk;
    char *buffer;
    int bufferSize;
    bool findResult;
    bool finish;
    string cmd;

    mutex mtx;
    condition_variable cv;

    DataServer(string name);
    void upload();
    void download();
    void downloadChunk();
    void find();
    void operator()();
};

#endif
