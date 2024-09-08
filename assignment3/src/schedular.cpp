#include<iostream>
#include<fstream> // for file reading
#include<sstream>  // for using iss
#include<vector>
#include<queue>
using namespace std;

class Process{
    public:
        int processNumber;  
        int arrivalTime;
        vector<int> bursts;

        Process(int processNumber, int arrivalTime, vector<int>& bursts){
            processNumber = processNumber;
            arrivalTime = arrivalTime;
            bursts = bursts;
        }
};

queue<Process> readyQueue; 
queue<Process> waitingQueue;

class Schedular{
    public:

        void addprocess(int processNumber, int arrivalTime, vector<int>& bursts, vector<Process> processList){
            Process p(processNumber, arrivalTime, bursts);
            processList.push_back(p); 
            readyQueue.push(p); // when new process arrives is pushed into ready queue
            cout << "Added process " << processNumber << " to the processList at arrival time " << arrivalTime << endl;
        }

        void load_info_from_file(string &filename){
            ifstream file(filename);
            string burst_info; // string containing info of arrival time , <cpu_burst>, <i/o_burst> ....

            int processNumber = 1; // numbering of process starts with 1
            vector<Process> processList; // list of processes

            while(getline(file, burst_info)){
                if(!burst_info.empty() && burst_info[0] != '<'){
                    istringstream iss(burst_info);

                    int arrivalTime;
                    iss >> arrivalTime; // first arg is arrival time 

                    vector<int> bursts; // vector containing info of <cpu_burst>, <i/o_burst> ....
                    int burst;
                    while(iss >> burst){
                        bursts.push_back(burst);
                    }

                    addprocess(processNumber, arrivalTime, bursts, processList);
                    processNumber++;
                }
            }

            file.close();  // close file once every process is pushed into ready queue for first time 
        }

};


int main(int argc, char **argv) {
    string process_info_file = argv[1];
    Schedular s;
    s.load_info_from_file(process_info_file);

    return 0;
}