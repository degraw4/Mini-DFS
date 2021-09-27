#include <iostream>
#include <fstream>
#include <thread>
#include <algorithm>
#include "dataserver.h"
using namespace std;

int chunkSize = 2 * 1024 * 1024;

DataServer::DataServer(string name): serverName(name), buffer(nullptr), finish(true), findResult(false){
    string cmd = "mkdir -p " + serverName;
    system(cmd.c_str());
}

void DataServer::operator()(){
    while (true){
        unique_lock<mutex> lock(mtx);
        while(this->finish){
            cv.wait(lock);
        }
        if (cmd == "up")
            upload();
        else if (cmd == "down")
            download();
        else if (cmd == "find")
            find();
        else if (cmd == "down-c")
            downloadChunk();
        this->finish = true;
        lock.unlock();
        cv.notify_all();
    }
}

void DataServer::upload(){
    int start = 0;
    ofstream os;
    while (start < bufferSize){
        int offset = start / chunkSize;
        string filePath = serverName + "/" + to_string(fileID) + "-" + to_string(offset);
        os.open(filePath);
        if (!os)
            cout << "Failed in creating file in data server: " << filePath << endl;
        os.write(&buffer[start], min(chunkSize, bufferSize - start));
        start += chunkSize;
        os.close();
    }
}

void DataServer::download(){
    int start = 0;
    buffer = new char[bufferSize];
    while (start < bufferSize){
        int offset = start / chunkSize;
        string filePath = serverName + "/" + to_string(fileID) + "-" + to_string(offset);
        ifstream is(filePath);
        if (!is){
            delete[] buffer;
            bufferSize = 0;
            break;
        }
        is.read(&buffer[start], min(chunkSize, bufferSize - start));
        start += chunkSize;
    }
}

void DataServer::downloadChunk(){
    buffer = new char[chunkSize];
    string filePath = serverName + "/" + to_string(fileID) + "-" + to_string(chunk);
    ifstream is(filePath);
    if (!is){
        delete[] buffer;
        bufferSize = 0;
    }
    else{
        is.read(buffer, min(chunkSize, bufferSize - chunkSize * chunk));
        bufferSize = is.tellg();
    }
}

void DataServer::find(){
    string filePath = serverName + "/" + to_string(fileID) + "-" + to_string(chunk);
    ifstream is(filePath);
    findResult = is ? true : false;
}
