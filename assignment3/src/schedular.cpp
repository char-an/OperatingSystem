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

    Process(int processNumber, int arrivalTime, vector<int>& bursts) {
        this->processNumber = processNumber;
        this->arrivalTime = arrivalTime;
        this->bursts = bursts;
    }

    int getArrivaltime() {
        return this->arrivalTime;
    }

    int getProcessNumber() {
        return this->processNumber;
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

    void addToReadyQueue(){
        // lambda function to sort processList on basis of Arrivaltime of process
        sort(processList.begin(), processList.end(), [](Process &a, Process &b){
            return a.getArrivaltime() < b.getArrivaltime();
        });

        for(Process p : processList){
            readyQueue.push(p);
        }
    }

    void FCFS(){
        while(!readyQueue.empty()){
            cout << "Number of process in scheduler " << readyQueue.size() << endl;
            Process p = readyQueue.front();
            cout << "Process to be run in CPU " << p.getProcessNumber() << endl;

            for(int i=0; i< p.bursts.size() - 1; i++){
                if(i%2 == 0){
                    cout << "cpu burst happens for " << p.bursts[i] << endl;
                }else{
                    cout << "i/o burst happens for " << p.bursts[i] << endl;
                }
            }
            readyQueue.pop();
        }
    }
};



int main(int argc, char** argv) {
    string process_info_file = argv[1];
    Schedular s;
    s.load_info_from_file(process_info_file);
    s.addToReadyQueue();
    s.FCFS();
    return 0;
}
