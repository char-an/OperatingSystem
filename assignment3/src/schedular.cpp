#include <iostream>
#include <fstream> // for file reading
#include <sstream> // for using iss
#include <vector>
#include <queue>
#include <chrono>
using namespace std;

class Process
{
public:
    int processNumber;
    int arrivalTime;
    vector<int> bursts;
    int remainingTime;
    int burstIdx;
    int waitingTime; // time when process is added to waiting queue
    int runningTime; // time when process actually starts running on cpu
    int timeSlice; // for round robin
    int terminatedTime;
    Process(int processNumber, int arrivalTime, vector<int> &bursts)
    {
        this->processNumber = processNumber;
        this->arrivalTime = arrivalTime;
        this->bursts = bursts;
        this->burstIdx = 0;
        this->remainingTime = bursts[this->burstIdx];
        this->waitingTime = 0;
        this->runningTime = 0;
        this->timeSlice = 0;
        this->terminatedTime = 0;
        // this->cpuAllocated = 1;
    }

    int getArrivaltime()
    {
        return this->arrivalTime;
    }

    int getCPUBurstNo()
    {
        return ((this->burstIdx) / 2 + 1);
    }

    int getProcessNumber()
    {
        return this->processNumber;
    }

    int getRemainingTime()
    {
        return this->remainingTime;
    }

    int getWaitingTime()
    {
        return this->waitingTime;
    }

    void setWaitingTime(int currTime)
    {
        this->waitingTime = currTime;
    }

    int getRunningTime()
    {
        return this->runningTime;
    }

    void setRunningTime(int currTime)
    {
        this->runningTime = currTime;
    }

    void burstChange()
    {
        this->burstIdx = this->burstIdx + 1;
        this->remainingTime = bursts[this->burstIdx];
    }

    void setTimeSlice(int n)
    {
        this->timeSlice = n;
    }

    bool isterminated()
    {
        if (this->remainingTime == -1)
        {
            return true;
        }
        return false;
    }

    void setTermTime(int currTime){
        this->terminatedTime = currTime;
    }
};

struct Comp
{
    bool operator()(const Process *a, const Process *b)
    {
        return a->remainingTime > b->remainingTime;
    }
};

struct Statement
{
    int processNumber;
    int burstIdx;
    int runningTime;
    int currTime;
    int CPUNumber;

    Statement(int processNumber, int burstIdx, int runningTime, int currTime, int CPUNumber)
    {
        this->processNumber = processNumber;
        this->burstIdx = burstIdx;
        this->runningTime = runningTime;
        this->currTime = currTime;
        this->CPUNumber = CPUNumber;
    }

    string formatMessage()
    {
        stringstream ss;
        ss << "CPU" << this->CPUNumber << " - P" << this->processNumber << ", " << this->burstIdx << "    " << this->runningTime << "    " << this->currTime;
        return ss.str();
    }
};

struct Print
{
    vector<Statement> messages;

    void addMessage(Statement &message)
    {
        messages.push_back(message);
    }

    void printAll()
    {
        for (auto &msg : messages)
        {
            cout << msg.formatMessage() << endl;
        }
        messages.clear();
    }

    void printCompletionTimes(vector<Process>  &processList)
    {
        cout << "\nCompletion times for all processes:" << endl;
        for (Process &p : processList)
        {
            cout << "P" << p.getProcessNumber() << " completed at time " << p.terminatedTime - p.arrivalTime + 1<< endl;
        }
    }

    void completionTime(vector<Process> &processList)
    {
        int sum =0;
        int max= 0;
        for (Process &p :processList){
            sum += p.terminatedTime - p.arrivalTime + 1;
            if (p.terminatedTime - p.arrivalTime + 1 > max){
                max = p.terminatedTime - p.arrivalTime + 1;
            }
        }

        int len = processList.size();
        double average = static_cast<double>(sum)/len;

        cout << "\nAverage Completion Time: " << average << endl;
        cout << "Maximum Completion Time: " << max << endl;
    }
};

queue<Process *> readyQueue;
queue<Process *> waitingQueue;
priority_queue<Process *, std::vector<Process *>, Comp> pq;

class Schedular
{
public:
    vector<Process> processList; // list of processes
    Print print;  // structure to store print statements

    void addprocess(int processNumber, int arrivalTime, vector<int> &bursts)
    {
        Process p(processNumber, arrivalTime, bursts);
        processList.push_back(p);
    }

    void load_info_from_file(string &filename)
    {
        ifstream file(filename);
        string burst_info; // string containing info of arrival time , <cpu_burst>, <i/o_burst> ....

        int processNumber = 1; // numbering of process starts with 1
        while (getline(file, burst_info))
        {
            if (!burst_info.empty() && burst_info[0] != '<')
            {
                istringstream iss(burst_info);

                int arrivalTime;
                iss >> arrivalTime; // first arg is arrival time

                vector<int> bursts; // vector containing info of <cpu_burst>, <i/o_burst> ....
                int burst;
                while (iss >> burst)
                {
                    bursts.push_back(burst);
                }

                addprocess(processNumber, arrivalTime, bursts);
                processNumber++;
            }
        }
        file.close(); // close file once every process is pushed into ready queue for first time
    }

    Process *getProcessByProcessNumber(int ProcessNumber)
    {
        for (Process &p : processList)
        {
            if (p.getProcessNumber() == ProcessNumber)
            {
                return &p;
            }
        }
        return nullptr;
    }

    void callPrint(){
        print.printAll();
    }

    void printCompletionTimes(){
        print.printCompletionTimes(processList);
    }
    
    void completionTime(){
        print.completionTime(processList);
    }

    void FIFO()
    {
        int currTime = 0;
        int runningProcessIndex0 = -1;
#ifdef TWO
        int runningProcessIndex1 = -1;
#endif

        bool finished_All = true;
        // cout << "CPU0" << endl;
        while (true)
        {
            finished_All = true;
            // first check is there any process is ProcessList that arrives at current time
            // if currTime = 0 and some process's arrival time is also 0, then we add that process in readyQueue
            for (Process &p : processList)
            {
                if (p.getArrivaltime() == currTime)
                {
                    readyQueue.push(&p); // Process is added to the ready queue
                }
            }

            if (!waitingQueue.empty())
            {
                queue<Process *> temp;
                while (!waitingQueue.empty())
                {
                    Process *p = waitingQueue.front();
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime() >= p->getRemainingTime())
                    {
                        p->burstChange();
                        if (!p->isterminated())
                            readyQueue.push(p);
                        else
                            p->setTermTime(currTime);   //completion time
                    }
                    else
                    {
                        temp.push(p);
                    }
                }
                waitingQueue = temp;
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (runningProcessIndex0 == -1 && !readyQueue.empty())
            {
                Process *p = readyQueue.front();
                readyQueue.pop();
                p->setRunningTime(currTime);
                runningProcessIndex0 = p->getProcessNumber(); // Update the running process
            }
#ifdef TWO
            if (runningProcessIndex1 == -1 && !readyQueue.empty())
            {
                Process *p = readyQueue.front();
                readyQueue.pop();
                p->setRunningTime(currTime);
                runningProcessIndex1 = p->getProcessNumber(); // Update the running process
            }
#endif

            // third now process the running process
            if (runningProcessIndex0 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex0); // Now returns a pointer
                runningProcess->remainingTime--;
                if (runningProcess->getRemainingTime() == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 0);
                    print.addMessage(s);
                    runningProcessIndex0 = -1;
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }
            }
#ifdef TWO
            if (runningProcessIndex1 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex1); // Now returns a pointer
                runningProcess->remainingTime--;
                if (runningProcess->getRemainingTime() == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 1);
                    print.addMessage(s);
                    runningProcessIndex1 = -1;
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }
            }
#endif

            for (Process &p : processList)
            {
                if (!p.isterminated())
                {
                    finished_All = false;
                }
            }

            if (finished_All)
            {
                break; // if cpu and i/o burst of all processes are completed then we break from while loop
            }

            currTime++;
        }
        // int makeSpan = currTime - 0;
        // cout << "\nFIFO" << endl;
        // cout << "Makespan: " << makeSpan << endl;
    }

    void SJF()
    {
        int currTime = 0;
        int runningProcessIndex0 = -1;
#ifdef TWO
        int runningProcessIndex1 = -1;
#endif
        bool finished_All = true;
        while (true)
        {
            finished_All = true;
            // first check is there any process is ProcessList that arrives at current time
            for (Process &p : processList)
            {
                if (p.getArrivaltime() == currTime)
                {
                    pq.push(&p);
                }
            }

            if (!waitingQueue.empty())
            {
                queue<Process *> temp;
                while (!waitingQueue.empty())
                {
                    Process *p = waitingQueue.front();
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime() == p->getRemainingTime())
                    {
                        p->burstChange();
                        if (!p->isterminated())
                            pq.push(p);
                        else
                        p->setTermTime(currTime);
                    }
                    else
                    {
                        temp.push(p);
                    }
                }
                waitingQueue = temp;
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (runningProcessIndex0 == -1 && !pq.empty())
            {
                Process *p = pq.top();
                pq.pop();
                p->setRunningTime(currTime);
                runningProcessIndex0 = p->getProcessNumber(); // Update the running process
            }
#ifdef TWO
            if (runningProcessIndex1 == -1 && !pq.empty())
            {
                Process *p = pq.top();
                pq.pop();
                p->setRunningTime(currTime);
                runningProcessIndex1 = p->getProcessNumber(); // Update the running process
            }
#endif

            // third now process the running process
            if (runningProcessIndex0 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex0); // Now returns a pointer
                runningProcess->remainingTime--;
                if (runningProcess->getRemainingTime() == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex0 = -1;
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 0);
                    print.addMessage(s);
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }
            }

#ifdef TWO
            if (runningProcessIndex1 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex1); // Now returns a pointer
                runningProcess->remainingTime--;
                if (runningProcess->getRemainingTime() == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex1 = -1;
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 1);
                    print.addMessage(s);
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }
            }
#endif

            for (Process &p : processList)
            {
                if (!p.isterminated())
                {
                    finished_All = false;
                }
            }

            if (finished_All)
            {
                break; // if cpu and i/o burst of all processes are completed then we break from while loop
            }

            currTime++;
        }
        // int makeSpan = currTime - 0;
        // cout << "\nSJF" << endl;
        // cout << "Makespan: " << makeSpan << endl;
    }

    void SRTF()
    {
        int currTime = 0;
        int runningProcessIndex0 = -1;
#ifdef TWO
        int runningProcessIndex1 = -1;
#endif
        bool finished_All = true;

        while (true)
        {
            finished_All = true;
            bool preemt = false;
            // first check is there any process is ProcessList that arrives at current time
            for (Process &p : processList)
            {
                if (p.getArrivaltime() == currTime)
                {
                    pq.push(&p);
                    preemt = true;
                }
            }

            if (!waitingQueue.empty())
            {
                queue<Process *> temp;
                while (!waitingQueue.empty())
                {
                    Process *p = waitingQueue.front();
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime() == p->getRemainingTime() + 1)
                    {
                        p->burstChange();
                        if (!p->isterminated())
                            pq.push(p);
                        else
                            p->setTermTime(currTime);
                        preemt = true;
                        //TODO: check if it should be inside if loop
                    }
                    else
                    {
                        temp.push(p);
                    }
                }
                waitingQueue = temp;
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (runningProcessIndex0 == -1 && !pq.empty())
            {
                Process *p = pq.top();
                pq.pop();
                p->setRunningTime(currTime);
                runningProcessIndex0 = p->getProcessNumber(); // Update the running process
            }
#ifdef TWO
            if (runningProcessIndex1 == -1 && !pq.empty())
            {
                Process *p = pq.top();
                pq.pop();
                p->setRunningTime(currTime);
                runningProcessIndex1 = p->getProcessNumber(); // Update the running process
            }
#endif

            // third now process the running process
            if (runningProcessIndex0 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex0); // Now returns a pointer

                if (preemt && runningProcessIndex0 != -1 && !pq.empty())
                {
                    Process *topProcess = pq.top();
                    if (topProcess->getRemainingTime() < runningProcess->getRemainingTime())
                    {
                        // put current process back to pq
                        //cout << "CPU0 - P" << runningProcess->getProcessNumber() << "," << runningProcess->getCPUBurstNo() << "    " << runningProcess->getRunningTime() << "    " << currTime - 1 << endl;
                        // Why currTime - 1 ?? -- Parikshit
                        Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime-1, 0);
                        print.addMessage(s);
                        pq.push(runningProcess);
                        runningProcess = pq.top();
                        pq.pop();
                        runningProcess->setRunningTime(currTime);
                        runningProcessIndex0 = runningProcess->getProcessNumber();
                    }
                }

                runningProcess->remainingTime--;

                if (runningProcess->remainingTime == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex0 = -1;
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 0);
                    print.addMessage(s);
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }
            }
#ifdef TWO
            if (runningProcessIndex1 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex1); // Now returns a pointer

                if (preemt && runningProcessIndex1 != -1 && !pq.empty())
                {
                    Process *topProcess = pq.top();
                    if (topProcess->getRemainingTime() < runningProcess->getRemainingTime())
                    {
                        // put current process back to pq
                        Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime-1, 1);
                        print.addMessage(s);
                        pq.push(runningProcess);
                        runningProcess = pq.top();
                        pq.pop();
                        runningProcess->setRunningTime(currTime);
                        runningProcessIndex1 = runningProcess->getProcessNumber();
                    }
                }

                runningProcess->remainingTime--;

                if (runningProcess->remainingTime == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex1 = -1;
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime - 1, 1);
                    print.addMessage(s);
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }

            }
#endif

            for (Process &p : processList)
            {
                if (!p.isterminated())
                {
                    finished_All = false;
                }
            }

            if (finished_All)
            {
                break; // if cpu and i/o burst of all processes are completed then we break from while loop
            }

            currTime++;
        }
        // int makeSpan = currTime - 0;
        // cout << "\nSRTF" << endl;
        // cout << "Makespan: " << makeSpan << endl;
    }

    void RR()
    {
        int timeQuantum = 50;
        int currTime = 0;
        int runningProcessIndex0 = -1;
#ifdef TWO
        int runningProcessIndex1 = -1;
#endif
        bool finished_All = true;

        while (true)
        {
            finished_All = true;
            // first check is there any process is ProcessList that arrives at current time
            for (Process &p : processList)
            {
                if (p.getArrivaltime() == currTime)
                {
                    readyQueue.push(&p);
                }
            }

            if (!waitingQueue.empty())
            {
                queue<Process *> temp;
                while (!waitingQueue.empty())
                {
                    Process *p = waitingQueue.front();
                    waitingQueue.pop();
                    if (currTime - p->getWaitingTime() == p->getRemainingTime() + 1)
                    {
                        p->burstChange();
                        if (!p->isterminated())
                            readyQueue.push(p);
                        else
                            p->setTermTime(currTime);
                    }
                    else
                    {
                        temp.push(p);
                    }
                }
                waitingQueue = temp;
            }

            // second check if no process is running currently, we will remove first process in readyqueue, now that process is in running state
            if (runningProcessIndex0 == -1 && !readyQueue.empty())
            {
                Process *p = readyQueue.front();
                readyQueue.pop();
                p->setRunningTime(currTime);
                p->setTimeSlice(timeQuantum);
                runningProcessIndex0 = p->getProcessNumber(); // Update the running process
            }
#ifdef TWO
            if (runningProcessIndex1 == -1 && !readyQueue.empty())
            {
                Process *p = readyQueue.front();
                readyQueue.pop();
                p->setRunningTime(currTime);
                p->setTimeSlice(timeQuantum);
                runningProcessIndex1 = p->getProcessNumber(); // Update the running process
            }
#endif

            // third now process the running process
            if (runningProcessIndex0 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex0); // Now returns a pointer

                runningProcess->remainingTime--;
                runningProcess->timeSlice--;

                if (runningProcess->remainingTime == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex0 = -1;
                    // TODO: same as for fifo
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 0);
                    print.addMessage(s);
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }
                else if (runningProcess->timeSlice == 0 && !readyQueue.empty())
                {
                    // put current process back to readyQueue
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 0);
                    print.addMessage(s);
                    readyQueue.push(runningProcess);
                    runningProcessIndex0 = -1;
                }
            }

#ifdef TWO
            if (runningProcessIndex1 != -1)
            {
                Process *runningProcess = getProcessByProcessNumber(runningProcessIndex1); // Now returns a pointer

                runningProcess->remainingTime--;
                runningProcess->timeSlice--;

                if (runningProcess->remainingTime == 0)
                {
                    runningProcess->setWaitingTime(currTime);
                    runningProcess->burstChange();
                    runningProcessIndex1 = -1;
                    // TODO: same as for fifo
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 1);
                    print.addMessage(s);
                    if (!runningProcess->isterminated())
                        waitingQueue.push(runningProcess); // Push pointer to waitingQueue
                    else
                        runningProcess->setTermTime(currTime);
                }
                else if (runningProcess->timeSlice == 0 && !readyQueue.empty())
                {
                    // put current process back to readyQueue
                    Statement s(runningProcess->getProcessNumber(), runningProcess->getCPUBurstNo(), runningProcess->getRunningTime(), currTime, 1);
                    print.addMessage(s);
                    readyQueue.push(runningProcess);
                    runningProcessIndex1 = -1;
                }
            }
#endif

            for (Process &p : processList)
            {
                if (!p.isterminated())
                {
                    finished_All = false;
                }
            }

            if (finished_All)
            {
                break; // if cpu and i/o burst of all processes are completed then we break from while loop
            }

            currTime++;
        }
        // int makeSpan = currTime - 0;
        // cout << "\nRR" << endl;
        // cout << "Makespan: " << makeSpan << endl;
    }
};

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        cout << "please enter correct number of arguments" << endl;
        return 1;
    }

    string process_info_file = argv[2];
    string algorithm = argv[1];
    Schedular s;
    s.load_info_from_file(process_info_file);

    const auto start = chrono::steady_clock::now();

    if (algorithm == "FIFO")
        s.FIFO();
    else if (algorithm == "SJF")
        s.SJF();
    else if (algorithm == "SRTF")
        s.SRTF();
    else if (algorithm == "RR")
        s.RR();

    const auto end = chrono::steady_clock::now();
    const chrono::duration<float> elapsedtime = end - start;

    cout << "Elapsed seconds: " << elapsedtime.count() << endl;

    s.callPrint();
    s.printCompletionTimes();
    s.completionTime();

    return 0;
}