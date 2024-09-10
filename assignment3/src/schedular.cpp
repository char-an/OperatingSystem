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

    Process(int processNumber, int arrivalTime, vector<int>& bursts) {
        this->processNumber = processNumber;
        this->arrivalTime = arrivalTime;
        this->bursts = bursts;
        this->remainingTime = bursts[0];
    }

    int getArrivaltime() {
        return this->arrivalTime;
    }

    int getProcessNumber() {
        return this->processNumber;
    }

    void decreaseRemainingTime(){
        this->remainingTime--;
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

    void FCFS(){
        int currTime = 0;
        int runningProcessIndex = -1;
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

            // third if there is running process, decrease its remaining(cpu burst) time by 1
            if(runningProcessIndex != -1){
                Process& runningProcess = getProcessByProcessNumber(runningProcessIndex); 
                runningProcess.decreaseRemainingTime();  // decrease remainimg time of the the process

                if(runningProcess.remainingTime == 0){
                    // cpu burst is over, so add process to waiting queue for i/o bursts
                    cout << "Time " << currTime << ": Process " << runningProcess.getProcessNumber() << " arrives in waiting queue.\n";
                    waitingQueue.push(runningProcess);  
                    runningProcessIndex = -1;  
                }
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
    s.FCFS();
    return 0;
}
