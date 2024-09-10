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
    Process(int processNumber, int arrivalTime, vector<int>& bursts) {
        this->processNumber = processNumber;
        this->arrivalTime = arrivalTime;
        this->bursts = bursts;
        this->burstIdx = 0;
        this->remainingTime = bursts[this->burstIdx];
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
        return this->remainingTime;
    }

    void setWaitingTime(int currTime){
        this->waitingTime = currTime;
    }
    void burstChange(){
        this->burstIdx++;
        this->remainingTime = bursts[this->burstIdx];
    }

    bool isterminated(){
        if(burstIdx >= bursts.size() - 1){
            return true;
        }
    }

};

class CPU{
public:
    bool isCPU_Available;
    int currentProcess; // number of current running on cpu
    CPU(){
        this->isCPU_Available = true;
    }

    bool check_cpu_avail(){
        return this->isCPU_Available;
    }

    void set_cpu_avail(bool val){
        this->isCPU_Available = val;
    }
};

class Timer{
public:
    int time;

    Timer(){
        this->time = 0;
    }
    void start(){
        time++;
    }
};


queue<Process> readyQueue; 
queue<Process> waitingQueue;

class Schedular {
public:
    vector<Process> processList;  // list of processes

    void addprocess(int processNumber, int arrivalTime, vector<int>& bursts, vector<Process>& processList) {
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

                addprocess(processNumber, arrivalTime, bursts, processList);
                processNumber++;
            }
        }
        file.close();  // close file once every process is pushed into ready queue for first time
    }

    Process& getProcessByProcessNumber(int ProcessNumber){
        for(Process& p : processList){
            if(p.getProcessNumber() == ProcessNumber){
                return p;
            }
        }
    }

    void removeProcessFromWaitingQueue(queue<Process>& q, Process p) {
        queue<Process> temp;
        while (!q.empty()) {
            if (q.front().getProcessNumber() != p.getProcessNumber()) {
                temp.push(q.front());
            }
            q.pop();  
        }
        q = temp;
    }

    void FIFO(){
        int currTime = 0;
        int runningProcessIndex = -1;
        // int waitingProcessIndex = -1;
        bool finished_All = true;
        while(true){
            // first check is there any process is ProcessList that arrives at current time
            // if currTime = 0 and some process's arrival time is also 0, then we add that process in readyQueue
            for(Process& p : processList){
                if(p.getArrivaltime() == currTime){
                    cout << "Time " << currTime << ": Process " << p.getProcessNumber() << " arrives in ready queue.\n";
                    readyQueue.push(p);  // Process is added to the ready queue
                }
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if(runningProcessIndex == -1 && !readyQueue.empty()){
                Process p = readyQueue.front();
                readyQueue.pop();
                cout << "Time " << currTime << ": Process " << p.getProcessNumber() << " gets removed from ready queue and is now in running state.\n";
                runningProcessIndex = p.getProcessNumber();
            }

            // third now process the running process
            if(runningProcessIndex != -1){
                Process& runningProcess = getProcessByProcessNumber(runningProcessIndex); 
                if(currTime == runningProcess.getRemainingTime()){
                    cout << "CPU burst of process " << runningProcess.getProcessNumber() << " is over at time :" << currTime << endl;
                    waitingQueue.push(runningProcess);
                    runningProcess.setWaitingTime(currTime);
                    runningProcess.burstChange();
                    runningProcessIndex = -1;
                }
            }

            if(!waitingQueue.empty()){
                queue<Process> waitingQueue_copy = waitingQueue;
                while(!waitingQueue_copy.empty()){
                    Process p = waitingQueue_copy.front();
                    waitingQueue_copy.pop();
                    if(p.remainingTime == currTime - p.getWaitingTime()){
                        removeProcessFromWaitingQueue(waitingQueue, p);
                        cout << "Time " << currTime << ": Process " << p.getProcessNumber() << " popped from waiting queue and pushed to ready queue.\n";
                        readyQueue.push(p);
                    }
                }
            }


            for(Process& p : processList){
                if(!p.isterminated()){
                    finished_All = false;
                }
            }

            if(finished_All)
                break;

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
