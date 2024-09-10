#include <iostream>
#include <fstream>  // for file reading
#include <sstream>  // for using iss
#include <vector>
#include <queue>
#include<algorithm> // for using sort
using namespace std;

class Process {
public:
    int processNumber;
    int arrivalTime;
    vector<int> bursts;
    int remainingTime;
    int burstIdx;
    int waitingTime; // time when process is added to waiting queue
    int runningTime; // time when process actually starts running on cpu 
    Process(int processNumber, int arrivalTime, vector<int>& bursts) {
        this->processNumber = processNumber;
        this->arrivalTime = arrivalTime;
        this->bursts = bursts;
        this->burstIdx = 0;
        this->remainingTime = bursts[this->burstIdx];
        this->waitingTime = 0;
    }

    int getArrivaltime() {
        return this->arrivalTime;
    }

    int getProcessNumber() {
        return this->processNumber;
    }

    int getRemainingTime(){
        return this->remainingTime;
    }

    int getWaitingTime(){
        return this->waitingTime;
    }

    void setWaitingTime(int currTime){
        this->waitingTime = currTime;
    }

    int getRunningTime(){
        return this->runningTime;
    }

    void setRunningTime(int currTime){
        this->runningTime = currTime;
    }

    void burstChange(){
        this->burstIdx = this->burstIdx + 1;
        this->remainingTime = bursts[this->burstIdx];
    }

    bool isterminated(){
        return burstIdx >= bursts.size() - 1;
    }
};

queue<Process*> readyQueue;
queue<Process*> waitingQueue;

class Schedular {
public:
    vector<Process> processList;  // list of processes

    void addprocess(int processNumber, int arrivalTime, vector<int>& bursts) {
        Process p(processNumber, arrivalTime, bursts);
        processList.push_back(p);
    }

    void load_info_from_file(string& filename) {
        ifstream file(filename);
        string burst_info;  // string containing info of arrival time , <cpu_burst>, <i/o_burst> ....

        int processNumber = 1;  // numbering of process starts with 1
        while (getline(file, burst_info)) {
            if (!burst_info.empty() && burst_info[0] != '<') {
                istringstream iss(burst_info);

                int arrivalTime;
                iss >> arrivalTime;  // first arg is arrival time

                vector<int> bursts;  // vector containing info of <cpu_burst>, <i/o_burst> ....
                int burst;
                while (iss >> burst) {
                    bursts.push_back(burst);
                }

                addprocess(processNumber, arrivalTime, bursts);
                processNumber++;
            }
        }
        file.close();  // close file once every process is pushed into ready queue for first time
    }

    Process* getProcessByProcessNumber(int ProcessNumber){
        for(Process& p : processList){
            if(p.getProcessNumber() == ProcessNumber){
                return &p;
            }
        }
        return nullptr;
    }

    void FIFO(){
        int currTime = 0;
        int runningProcessIndex = -1;
        // int waitingProcessIndex = -1;
        bool finished_All = true;
        while(true){
            finished_All = true;
            // first check is there any process is ProcessList that arrives at current time
            // if currTime = 0 and some process's arrival time is also 0, then we add that process in readyQueue
            for(Process& p : processList){
                if(p.getArrivaltime() == currTime){
                    cout << "Time " << currTime << ": Process " << p.getProcessNumber() << " arrives in ready queue.\n";
                    readyQueue.push(&p);  // Process is added to the ready queue
                }
            }

            if (!waitingQueue.empty()) {
                queue<Process*> temp;
                while (!waitingQueue.empty()) {
                    Process* p = waitingQueue.front();  
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime() >= p->getRemainingTime()) {
                        cout << "Time " << currTime << ": Process " << p->getProcessNumber() << " popped from waiting queue and pushed to ready queue.\n";
                        p->burstChange();
                        readyQueue.push(p);  
                    } else {
                        temp.push(p);  
                    }
                }
                waitingQueue = temp;
            }


            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (!readyQueue.empty()) {
                Process* p = readyQueue.front();  
                readyQueue.pop();                 
                p->setRunningTime(currTime);     
                cout << "Time " << currTime << ": Process " << p->getProcessNumber() << " gets removed from ready queue and is now in running state.\n";
                runningProcessIndex = p->getProcessNumber();  // Update the running process
            }

            

            // third now process the running process
            if(runningProcessIndex != -1){
                Process* runningProcess = getProcessByProcessNumber(runningProcessIndex);  // Now returns a pointer
                if (currTime - runningProcess->getRunningTime() == runningProcess->getRemainingTime()) {
                    cout << "CPU burst of process " << runningProcess->getProcessNumber() << " is over at time: " << currTime << endl;
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex = -1;
                    waitingQueue.push(runningProcess);  // Push pointer to waitingQueue
                }

            }

            
            for(Process& p : processList){
                if(!p.isterminated()){
                    finished_All = false;
                }
            }

            if(finished_All){
                break;
            }
            currTime++;
        }
    }

    void SJF(){

    }

    void STRF(){

    }

    void RR(){

    }
};



int main(int argc, char** argv) {
    string process_info_file = argv[1];
    Schedular s;
    s.load_info_from_file(process_info_file);
    s.FIFO();
    return 0;
}
