#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include "nameserver.h"
#include "utils.h"
using namespace std;

NameServer::NameServer(int number) : serverNumber(number), fileNumber(0) {
    directory["/"] = make_pair(0, 0);
}

vector<string> NameServer::shell(){
    cout << "\033[1m\033[32mMini-DFS: \033[0m";
    string cmd, tmp;
    vector<string> command;
    getline(cin, cmd);
    stringstream stringStream(cmd);
    while (stringStream >> tmp)
        command.push_back(tmp);
    return command;
}

void NameServer::list(){
    map<string, pair<int, int>>::iterator it;
    for(it = directory.begin(); it != directory.end(); it++){
        cout << it->second.first << "\t\t" 
        << (int)ceil(1.0 * it->second.second / (2 * 1024 * 1024)) << "\t\t" 
        << it->first <<endl;
    }
}

void NameServer::operator()(){
    while (true){
        char *buffer = nullptr;
        vector<string> command = shell();
        int len = command.size();
        ifstream is;
        MD5 md5;
        // empty
        if (!len)
            continue;
        // quit
        else if (command[0] == "quit" || command[0] == "exit")
            exit(0);
        // ls
        else if (command[0] == "ls"){
            if (len != 1)
                cout << "Usage: [ls]\n"<< "List all files in server\n";
            else{
                cout << "Total: " << fileNumber << endl;
                cout << "File ID\t\tChunk Number\tFile Name\n";
                list();
            }
            continue;
        }
        // find
        else if (command[0] == "find"){
            if (len != 3){
                cout << "Usage: [find] [fileID] [chunk]\n"<< "Find chunk of fileID in dataServers\n";
                continue;
            }
            else{
                bool flag = false;
                for(int i = 0; i < command[1].size(); i++){
                    if(command[1][i] < '0' ||command[1][i] > '9'){
                        cout << "ERROR: FileID '" << command[1] << "' is not legal\n";
                        flag = true;
                        break;
                    }
                }
                if(flag)    
                    continue;
                for(int i = 0; i < command[2].size(); i++){
                    if(command[2][i] < '0' ||command[2][i] > '9'){
                        cout << "ERROR: Chunk number '" << command[2] << "' is not legal\n";
                        flag = true;
                        break;
                    }
                }
                if(flag)    
                    continue;
                for (int i = 0; i < 4; i++){
                    unique_lock<mutex> lock(dataServers[i]->mtx);
                    dataServers[i]->cmd = command[0];
                    dataServers[i]->fileID = stoi(command[1]);
                    dataServers[i]->chunk = stoi(command[2]);
                    dataServers[i]->finish = false;
                    lock.unlock();
                    dataServers[i]->cv.notify_all();
                }
            }
        }
        // upload
        else if (command[0] == "up"){
            if (len != 3){
                cout << "Usage: [up] [source] [destination]\n"
                     << "Upload file from source(local) to destination(server)\n";
                continue;
            }
            is.open(command[1], ifstream::ate | ifstream::binary);
            if (!is){
                cout << "Cannot open '" << command[1] <<"': No such file or directory\n";
                continue;
            }
            else if (directory.find(command[2]) != directory.end()){
                cout << "Cannot upload '" << command[2] <<"': File already exists\n";
                continue;
            }
            else{
                int splitIndex = command[2].find_last_of("/");
                string tmpPath = command[2].substr(0, splitIndex);
                if (directory.find(tmpPath) != directory.end()){
                    cout << "Cannot upload: '" << command[2] << "' is not a legal path\n";
                    continue;
                }
                int totalSize = is.tellg();
                buffer = new char[totalSize];
                is.seekg(0, is.beg);
                is.read(buffer, totalSize);
                fileNumber++;
                for (int i = 0; i < serverNumber; i++){
                    unique_lock<mutex> lock(dataServers[i]->mtx);
                    directory[command[2]] = make_pair(fileNumber, totalSize);
                    dataServers[i]->cmd = command[0];
                    dataServers[i]->fileID = fileNumber;
                    dataServers[i]->bufferSize = totalSize;
                    dataServers[i]->buffer = buffer;
                    dataServers[i]->finish = false;
                    lock.unlock();
                    dataServers[i]->cv.notify_all();
                }
            }
        }
        // download
        else if (command[0] == "down"){
            if(len != 3){
                cout << "Usage: [down] [source] [destination]\n"
                << "Download file from source(server) to destination(local)\n";
                continue;
            }
            else if(directory.find(command[1]) == directory.end()){
                cout << "Cannot find file '"<< command[1] <<"' in Mini-DFS\n";
                continue;
            }
            else{
                for(int i = 0; i < 4; i++){
                    unique_lock<mutex> lock(dataServers[i]->mtx);
                    dataServers[i]->cmd = command[0];
                    dataServers[i]->fileID = directory[command[1]].first;
                    dataServers[i]->bufferSize = directory[command[1]].second;
                    dataServers[i]->finish = false;
                    lock.unlock();
                    dataServers[i]->cv.notify_all();
                }
            }
        }
        // download chunk
        else if (command[0] == "down-c"){
            if(len != 4){
                cout << "Usage: [down-c] [fileID] [chunk] [destination]\n"
                << "Download chunk of fileID to destination(local)\n";
                continue;
            }
            else{
                bool flag = false;
                for(int i = 0; i < command[1].size(); i++){
                    if(command[1][i] < '0' ||command[1][i] > '9'){
                        cout << "ERROR: FileID '" << command[1] << "' is not legal\n";
                        flag = true;
                        break;
                    }
                }
                if(flag)    
                    continue;
                for(int i = 0; i < command[2].size(); i++){
                    if(command[2][i] < '0' ||command[2][i] > '9'){
                        cout << "ERROR: Chunk number '" << command[2] << "' is not legal\n";
                        flag = true;
                        break;
                    }
                }
                if(flag)    
                    continue;
                if(stoi(command[1]) > fileNumber){
                    cout << "Cannot find fileID '"<< command[1] <<"' in Mini-DFS\n";
                    continue;
                }
                for(int i = 0; i < 4; i++){
                    unique_lock<mutex> lock(dataServers[i]->mtx);
                    dataServers[i]->cmd = command[0];
                    dataServers[i]->fileID = stoi(command[1]);
                    dataServers[i]->chunk = stoi(command[2]);
                    dataServers[i]->finish = false;
                    lock.unlock();
                    dataServers[i]->cv.notify_all();
                }
            }
        }
        // others
        else
            cout << "Unknown command\n";

        // dataServer processing
        for (const auto &server : dataServers){
            unique_lock<mutex> lock(server->mtx);
            while(!server->finish){
                (server->cv).wait(lock);
            }
            lock.unlock();
            (server->cv).notify_all();
        }

        // nameServer respond
        // up
        if (command[0] == "up")
            cout << "Upload success, fileID is " << fileNumber << endl;
        // find
        else if (command[0] == "find"){
            for (int i = 0; i < 4; i++){
                if (dataServers[i]->findResult)
                    cout << "Found fileID: " << command[1] << ", chunk:" << command[2] << " at " << dataServers[i]->serverName << endl;
                else
                    cout << "ERROR: Cannot find fileID: " << command[1] << ", chunk:" << command[2] << " at " << dataServers[i]->serverName << endl;
            }
        }
        // down
        else if (command[0] == "down"){
            string nowCheck, preCheck;
            bool successDownload = false;
            bool successCheck = false;
            for (int i = 0; i < 4; i++){
                if (dataServers[i]->bufferSize){
                    ofstream os;
                    os.open(command[2]);
                    if (!os)
                        cout << "Cannot download file. Please ensure local directory exists\n";
                    else{
                        successDownload = true;
                        os.write(dataServers[i]->buffer, dataServers[i]->bufferSize);
                        os.close();
                        md5.update(dataServers[i]->buffer, dataServers[i]->bufferSize);
                        md5.finalize();
                        nowCheck = md5.toString();
                        if (!preCheck.empty() && preCheck != nowCheck)
                            cout << "Error: inconsistent checksum for files from different data servers. File may be corrupted\n";
                        else if (!preCheck.empty() && preCheck == nowCheck){
                            successCheck = true;
                            cout << "MD5 checked\n";
                        }
                        preCheck = nowCheck;
                    }
                    delete[] dataServers[i]->buffer;
                }
            }
            if(!successDownload)
                cout << "Download failed\n";
            else if(!successCheck)
                cout << "Download success, MD5sum check failed\n";
            else
                cout << "Download success, MD5sum check success\n";
        }
        // down-c
        else  if  (command[0] == "down-c"){
            string nowCheck, preCheck;
            bool successDownload = false;
            bool successCheck = false;
            for (int i = 0; i < 4; i++){
                if (dataServers[i]->bufferSize){
                    ofstream os;
                    os.open(command[3]);
                    if (!os)
                        cout << "Cannot download file. Please ensure local directory exists\n";
                    else{
                        successDownload = true;
                        os.write(dataServers[i]->buffer, dataServers[i]->bufferSize);
                        os.close();
                        md5.update(dataServers[i]->buffer, dataServers[i]->bufferSize);
                        md5.finalize();
                        nowCheck = md5.toString();
                        if (!preCheck.empty() && preCheck != nowCheck)
                            cout << "Error: inconsistent checksum for files from different data servers. File may be corrupted\n";
                        else if (!preCheck.empty() && preCheck == nowCheck){
                            successCheck = true;
                            cout << "MD5 checked\n";
                        }
                        preCheck = nowCheck;
                    }
                    delete[] dataServers[i]->buffer;
                }
            }
            if(!successDownload)
                cout << "Download failed\n";
            else if(!successCheck)
                cout << "Download success, MD5sum check failed\n";
            else
                cout << "Download success, MD5sum check success\n";
        }

        delete[] buffer;
        is.close();
    }
}
