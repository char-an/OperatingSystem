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
    //int cpuAllocated;
    Process(int processNumber, int arrivalTime, vector<int>& bursts) {
        this->processNumber = processNumber;
        this->arrivalTime = arrivalTime;
        this->bursts = bursts;
        this->burstIdx = 0;
        this->remainingTime = bursts[this->burstIdx];
        this->waitingTime = 0;
        this->runningTime = 0;
        //this->cpuAllocated = 1;
    }

    int getArrivaltime() {
        return this->arrivalTime;
    }

    int getCPUBurstNo(){
        return ((this->burstIdx)/2 + 1);
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
        if(this->remainingTime == -1){
    	       return true;
    	 }
    	return false;
    }
};

struct Comp {
    bool operator()(const Process* a, const Process* b) {
        return a->remainingTime > b->remainingTime;
    }
};


queue<Process*> readyQueue;
queue<Process*> waitingQueue;
priority_queue<Process*, std::vector<Process*>, Comp> pq;

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
        bool finished_All = true;
        cout << "CPU0" << endl;
        while(true){
            finished_All = true;
            // first check is there any process is ProcessList that arrives at current time
            // if currTime = 0 and some process's arrival time is also 0, then we add that process in readyQueue
            for(Process& p : processList){
                if(p.getArrivaltime() == currTime){
                    readyQueue.push(&p);  // Process is added to the ready queue
                }
            }

            if (!waitingQueue.empty()) {
                queue<Process*> temp;
                while (!waitingQueue.empty()) {
                    Process* p = waitingQueue.front();
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime()>= p->getRemainingTime()) {
                        p->burstChange();
                        if(!p->isterminated())
                        	readyQueue.push(p);
                    } else {
                        temp.push(p);
                    }
                }
                waitingQueue = temp;
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (runningProcessIndex == -1 && !readyQueue.empty()) {
                Process* p = readyQueue.front();
                readyQueue.pop();
                p->setRunningTime(currTime);
                runningProcessIndex = p->getProcessNumber();  // Update the running process
            }

            // third now process the running process
            if(runningProcessIndex != -1){
                Process* runningProcess = getProcessByProcessNumber(runningProcessIndex);  // Now returns a pointer
                runningProcess->remainingTime--;
                if (runningProcess->getRemainingTime()==0) {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    //TODO: after comma it should be ith cpu burst of the process
                    cout << "P" << runningProcess->getProcessNumber() << "," << runningProcess->getCPUBurstNo() << "    " << runningProcess->getRunningTime() << "    "<< currTime << endl;
                    runningProcessIndex = -1;
                    if(!runningProcess->isterminated())
                    	waitingQueue.push(runningProcess);  // Push pointer to waitingQueue
                }
            }

            for(Process& p : processList){
                if(!p.isterminated()){
                    finished_All = false;
                }
            }

            if(finished_All){
                break;  // if cpu and i/o burst of all processes are completed then we break from while loop 
            }

            currTime++;
        }
    }

    void SJF(){
        int currTime = 0;
        int runningProcessIndex = -1;
        bool finished_All = true;
        while(true){
            finished_All = true;
            // first check is there any process is ProcessList that arrives at current time
            for(Process& p : processList){
                if(p.getArrivaltime() == currTime){
                    pq.push(&p); 
                }
            }

            if (!waitingQueue.empty()) {
                queue<Process*> temp;
                while (!waitingQueue.empty()) {
                    Process* p = waitingQueue.front();
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime()== p->getRemainingTime()) {
                        p->burstChange();
                        if(!p->isterminated())
                        	pq.push(p);
                    } else {
                        temp.push(p);
                    }
                }
                waitingQueue = temp;
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (runningProcessIndex == -1 && !pq.empty()) {
                Process* p = pq.top();
                pq.pop();
                p->setRunningTime(currTime);
                runningProcessIndex = p->getProcessNumber();  // Update the running process
            }

            // third now process the running process
            if(runningProcessIndex != -1){
                Process* runningProcess = getProcessByProcessNumber(runningProcessIndex);  // Now returns a pointer
                runningProcess->remainingTime--;
                if (runningProcess->getRemainingTime() == 0) {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex = -1;
                    //TODO: same as for fifo
                    cout << "P" << runningProcess->getProcessNumber() << "," << runningProcess->getCPUBurstNo() << "    " << runningProcess->getRunningTime() << "    "<< currTime << endl;
                    if(!runningProcess->isterminated())
                    	waitingQueue.push(runningProcess);  // Push pointer to waitingQueue
                }
            }

            for(Process& p : processList){
                if(!p.isterminated()){
                    finished_All = false;
                }
            }

            if(finished_All){
                break;  // if cpu and i/o burst of all processes are completed then we break from while loop 
            }

            currTime++;
        }
    }

    void STRF(){
        int currTime = 0;
        int runningProcessIndex = -1;
        bool finished_All = true;

        while(true){
            finished_All = true;
            bool preemt = false;
            // first check is there any process is ProcessList that arrives at current time
            for(Process& p : processList){
                if(p.getArrivaltime() == currTime){
                    pq.push(&p); 
                    preemt = true;
                }
            }

            if (!waitingQueue.empty()) {
                queue<Process*> temp;
                while (!waitingQueue.empty()) {
                    Process* p = waitingQueue.front();
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime() == p->getRemainingTime() +1) {
                        p->burstChange();
                        if(!p->isterminated())
                        	pq.push(p);
                            preemt = true;
                    } else {
                        temp.push(p);
                    }
                }
                waitingQueue = temp;
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (runningProcessIndex == -1 && !pq.empty()) {
                Process* p = pq.top();
                pq.pop();
                p->setRunningTime(currTime);
                runningProcessIndex = p->getProcessNumber();  // Update the running process
            }

            // third now process the running process
            if(runningProcessIndex != -1){
                Process* runningProcess = getProcessByProcessNumber(runningProcessIndex);  // Now returns a pointer

                if(preemt && runningProcessIndex != -1 && !pq.empty()){
                    Process* topProcess = pq.top();
                    if (topProcess->getRemainingTime() < runningProcess->getRemainingTime()){
                        //put current process back to pq
                        cout << "P" << runningProcess->getProcessNumber() << "," << runningProcess->getCPUBurstNo() << "    " << runningProcess->getRunningTime() << "    "<< currTime-1 << endl;
                        pq.push(runningProcess);
                        runningProcess = pq.top();
                        pq.pop();
                        runningProcess->setRunningTime(currTime);
                        runningProcessIndex = runningProcess->getProcessNumber();
                    }
         
                }

                runningProcess->remainingTime--;

                if(runningProcess->remainingTime == 0){
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex = -1;
                    //TODO: same as for fifo
                    cout << "P" << runningProcess->getProcessNumber() << "," << runningProcess->getCPUBurstNo() << "    " << runningProcess->getRunningTime() << "    "<< currTime << endl;
                    if(!runningProcess->isterminated())
                    	waitingQueue.push(runningProcess);  // Push pointer to waitingQueue
                }
                
                // runningProcess->remainingTime--;
            }

            for(Process& p : processList){
                if(!p.isterminated()){
                    finished_All = false;
                }
            }

            if(finished_All){
                break;  // if cpu and i/o burst of all processes are completed then we break from while loop 
            }

            currTime++;
        }
    }
    

    void RR(){

    }
};





int main(int argc, char** argv) {

    if(argc < 3){
        cout << "please enter correct number of arguments" << endl;
        return 1;
    }

    string process_info_file = argv[2];
    string algorithm = argv[1]; 
    Schedular s;
    s.load_info_from_file(process_info_file);
    
    if(algorithm=="FIFO")
        s.FIFO();
    else if(algorithm=="SJF")
        s.SJF();
    else if(algorithm=="SRTF")
        s.STRF();
    else if(algorithm=="RR")
        s.RR();

    return 0;
}