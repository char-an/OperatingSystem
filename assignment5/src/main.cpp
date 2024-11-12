#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include <bitset>
#include <cmath>
#include <queue>
#include <list>
#include <algorithm>

using namespace std;

int pageSize;
int noOfFrames;
int noOfFrames_per_p;
string replacementPolicy;
string allocationPolicy;
int k = 0;
int globalPageFault;
string toBinaryString(uint64_t num);

queue<int> fifoQueue;
list<int> lrulist;
// Vector for storing file info, used only in case of Optimal replacement policy
vector< pair<int, uint64_t> > info;
vector<int> frameVec;
int CurrIdx;

class Process
{
public:
    int ProcessId;
    unordered_map<uint64_t, uint64_t> PageTable;
    int localPageFault;
    queue<int> allocatedFrames;
    list<int> rallocatedFrames;
    list<int> lallocatedFrames;
    vector<int> oallocatedFrames;

    Process(int ProcessId)
    {
        this->ProcessId = ProcessId;
        this->localPageFault = 0;
    }

    void deleteMapping(uint64_t p)
    {
        PageTable.erase(p);
    }

    int getProcessNumber()
    {
        return ProcessId;
    }

    void incrementLocal()
    {
        localPageFault++;
    }

    int getLocal()
    {
        return localPageFault;
    }

    bool hasFreeFrame()
    {
        int limit = noOfFrames_per_p;
        if (replacementPolicy == "FIFO")
            return allocatedFrames.size() < limit;
        else if (replacementPolicy == "RANDOM")
            return rallocatedFrames.size() < limit;
        else if (replacementPolicy == "LRU")
            return lallocatedFrames.size() < limit;
        else if (replacementPolicy == "OPTIMAL")
            return oallocatedFrames.size() < limit;
        return false;
    }
};

class MemoryManager
{
public:
    vector<Process> ProcessList;
    map<int, pair<int, uint64_t> > PhysicalMemory; // key: frame, value: pid, page no.

    void allocateMemory(int ProcessId, uint64_t p, uint64_t f)
    {
        Process *process = getProcessByProcessNumber(ProcessId);
        if (process == nullptr)
        {
            Process newProcess(ProcessId);
            ProcessList.push_back(newProcess);
            process = &ProcessList.back();
        }
        process->PageTable[p] = f;
    }

    Process *getProcessByProcessNumber(int ProcessNumber)
    {
        for (Process &p : ProcessList)
            if (p.getProcessNumber() == ProcessNumber)
                return &p;
        return nullptr;
    }

    // Check if free frames present
    void checkFrames(int ProcessId, uint64_t p)
    {
        Process *process = getProcessByProcessNumber(ProcessId);
        int f = -1;
        if (allocationPolicy == "Global")
        {
            if (PhysicalMemory.size() < noOfFrames)
            {
                f = PhysicalMemory.size();
                PhysicalMemory[f] = make_pair(ProcessId, p);
                allocateMemory(ProcessId, p, f);
                addFrameToPolicy(f, process);
            }
            else
            {
                replaceFrameGlobal(ProcessId, p);
            }
        }
        else if (allocationPolicy == "Local")
        {
            if (process->hasFreeFrame())
            {
                f = PhysicalMemory.size();
                PhysicalMemory[f] = make_pair(ProcessId, p);
                allocateMemory(ProcessId, p, f);
                addFrameToLocalPolicy(f, process);
            }
            else
            {
                replaceFrameLocal(ProcessId, p, process);
            }
        }
    }

    void addFrameToPolicy(int frame, Process *process)
    {
        if (replacementPolicy == "FIFO")
            fifoQueue.push(frame);
        else if (replacementPolicy == "LRU")
            lrulist.push_back(frame);
        else if (replacementPolicy == "OPTIMAL")
            frameVec.push_back(frame);
    }

    void addFrameToLocalPolicy(int frame, Process *process)
    {
        if (replacementPolicy == "FIFO")
            process->allocatedFrames.push(frame);
        else if (replacementPolicy == "RANDOM")
            process->rallocatedFrames.push_back(frame);
        else if (replacementPolicy == "LRU")
            process->lallocatedFrames.push_back(frame);
        else if (replacementPolicy == "OPTIMAL")
            process->oallocatedFrames.push_back(frame);
    }

    void replaceFrameGlobal(int ProcessId, uint64_t p)
    {
        if (replacementPolicy == "FIFO")
            FIFO(ProcessId, p);
        else if (replacementPolicy == "LRU")
            LRU(ProcessId, p);
        else if (replacementPolicy == "RANDOM")
            RANDOM(ProcessId, p);
        else if (replacementPolicy == "OPTIMAL")
            OPTIMAL(ProcessId, p);
    }

    void replaceFrameLocal(int ProcessId, uint64_t p, Process *process)
    {
        if (replacementPolicy == "FIFO")
            FIFO_Local(ProcessId, p, process);
        else if (replacementPolicy == "LRU")
            LRU_Local(ProcessId, p, process);
        else if (replacementPolicy == "RANDOM")
            RANDOM_Local(ProcessId, p, process);
        else if (replacementPolicy == "OPTIMAL")
            OPTIMAL_Local(ProcessId, p, process);
    }

    void OPTIMAL_Local(int ProcessId, uint64_t p, Process *process)
    {
        int f = frameToBeRemovedLocal(process);
        if (f == -1)
        {
            cout << "Error , f == -1" << endl;
            return;
        }

        auto it = PhysicalMemory.find(f);
        if (it != PhysicalMemory.end())
        {
            deletePageTableMapping(it->second.first, it->second.second);
            PhysicalMemory.erase(f);
        }
        else
        {
            cout << "LOGIC ERROR !!" << endl;
        }

        auto iter = find(process->oallocatedFrames.begin(), process->oallocatedFrames.end(), f);

        if (iter != process->oallocatedFrames.end())
        {
            process->oallocatedFrames.erase(iter);
        }
        PhysicalMemory[f] = make_pair(ProcessId, p);
        allocateMemory(ProcessId, p, f);
        process->oallocatedFrames.push_back(f);
    }

    int frameToBeRemovedLocal(Process *process)
    {
        int resultantFrame = -1;
        int maxDist = -1;
        unordered_map<uint64_t, int> nextUsage;

        for (int i = info.size() - 1; i >= CurrIdx; i--)
        {
            auto iter = info[i];
            string binary = toBinaryString(iter.second);
            int dSize = log2(pageSize);
            int pSize = 64 - dSize;
            uint64_t p = stoull(binary.substr(0, pSize), nullptr, 2);

            nextUsage[p] = i;
        }

        for (int frame : process->oallocatedFrames)
        {
            auto it = PhysicalMemory.find(frame);
            int Dist = info.size();

            if (it != PhysicalMemory.end())
            {
                uint64_t currentPage = it->second.second;

                if (nextUsage.find(currentPage) != nextUsage.end() && nextUsage[currentPage] > CurrIdx)
                {
                    Dist = nextUsage[currentPage] - CurrIdx;
                }
            }

            if (Dist > maxDist)
            {
                maxDist = Dist;
                resultantFrame = frame;
            }
        }
        return resultantFrame;
    }

    void OPTIMAL(int ProcessId, uint64_t p)
    {
        // cout << "OPTIMAL" << endl;
        int f = frameToBeRemoved();
        if (f == -1)
        {
            cout << "Error , f == -1" << endl;
            return;
        }
        // cout << "replacement - process id : " << ProcessId << " frame no. : " << f << endl;
        auto it = PhysicalMemory.find(f);
        if (it != PhysicalMemory.end())
        {
            deletePageTableMapping(it->second.first, it->second.second);
            PhysicalMemory.erase(f);
        }
        else
        {
            cout << "LOGIC ERROR !!" << endl;
        }

        auto iter = find(frameVec.begin(), frameVec.end(), f);

        if (iter != frameVec.end())
        {
            frameVec.erase(iter);
        }
        PhysicalMemory[f] = make_pair(ProcessId, p);
        allocateMemory(ProcessId, p, f);
        frameVec.push_back(f);
    }

    int frameToBeRemoved()
    {
        int resultantFrame = -1;
        int maxDist = -1;
        unordered_map<uint64_t, int> nextUsage;

        for (int i = info.size() - 1; i >= CurrIdx; i--)
        {
            auto iter = info[i];
            string binary = toBinaryString(iter.second);
            int dSize = log2(pageSize);
            int pSize = 64 - dSize;
            uint64_t p = stoull(binary.substr(0, pSize), nullptr, 2);

            nextUsage[p] = i;
        }

        for (int frame : frameVec)
        {
            auto it = PhysicalMemory.find(frame);
            int Dist = info.size();

            if (it != PhysicalMemory.end())
            {
                uint64_t currentPage = it->second.second;

                if (nextUsage.find(currentPage) != nextUsage.end() && nextUsage[currentPage] > CurrIdx)
                {
                    Dist = nextUsage[currentPage] - CurrIdx;
                }
            }

            if (Dist > maxDist)
            {
                maxDist = Dist;
                resultantFrame = frame;
            }
        }
        return resultantFrame;
    }

    void FIFO(int ProcessId, uint64_t p)
    {
        int f = fifoQueue.front();      //frame to replace
        removeFrame(f, ProcessId, p);   //remove and allocate frame from pageTable and physical memory
        fifoQueue.pop();
        fifoQueue.push(f);
    }

    void FIFO_Local(int ProcessId, uint64_t p, Process *process)
    {
        int f = process->allocatedFrames.front();
        removeFrame(f, ProcessId, p);
        process->allocatedFrames.pop();
        process->allocatedFrames.push(f);
    }

    void LRU(int ProcessId, uint64_t p)
    {
        int f = lrulist.front();        //frame to replace
        removeFrame(f, ProcessId, p);   //remove and allocate frame from pageTable and physical memory
        lrulist.pop_front();
        lrulist.push_back(f);
    }

    void LRU_Local(int ProcessId, uint64_t p, Process *process)
    {
        int f = process->lallocatedFrames.front();
        removeFrame(f, ProcessId, p);
        process->lallocatedFrames.pop_front();
        process->lallocatedFrames.push_back(f);
    }

    void RANDOM(int ProcessId, uint64_t p)
    {
        int f = rand() % noOfFrames;
        removeFrame(f, ProcessId, p);    //remove and allocate frame from pageTable and physical memory
    }

    void RANDOM_Local(int ProcessId, uint64_t p, Process *process)
    {
        int allocatedSize = process->rallocatedFrames.size();
        int randomIndex = rand() % allocatedSize;

        auto it = process->rallocatedFrames.begin();
        std::advance(it, randomIndex);
        
        int frameToReplace = *it;
        process->rallocatedFrames.erase(it);
        removeFrame(frameToReplace, ProcessId, p);
        process->rallocatedFrames.push_back(frameToReplace);

    }

    //remove and allocate frame from pageTable aswell as physical memory
    void removeFrame(int frame, int ProcessId, uint64_t p)
    {
        auto it = PhysicalMemory.find(frame);
        if (it != PhysicalMemory.end())
        {
            deletePageTableMapping(it->second.first, it->second.second);
            PhysicalMemory.erase(frame);
            PhysicalMemory[frame] = make_pair(ProcessId, p);
            allocateMemory(ProcessId, p, frame);
        }
    }

    void deletePageTableMapping(int ProcessId, uint64_t p)
    {
        Process *process = getProcessByProcessNumber(ProcessId);
        if (process)
            process->deleteMapping(p);
    }

    void checkPageTable(int ProcessId, uint64_t logicalAddress)
    {
        Process *process = getProcessByProcessNumber(ProcessId);
        if (process == nullptr)
        {
            Process newProcess(ProcessId);
            ProcessList.push_back(newProcess);
            process = &ProcessList.back();
        }

        string binary = toBinaryString(logicalAddress);
        // cout << "Logical address : " << binary << endl;
        int dSize = log2(pageSize);
        int pSize = 64 - dSize;                                     // since 64bit system
        uint64_t p = stoull(binary.substr(0, pSize), nullptr, 2);   // change depending on page size
        uint64_t d = stoull(binary.substr(pSize, dSize), nullptr, 2); // change depending on page size

         // cout << "Page number : " << p << endl;
        // cout << "Offset : " << d << endl;

        if (process->PageTable.find(p) == process->PageTable.end())
        { // Page Fault
            globalPageFault++;
            process->incrementLocal();
            checkFrames(ProcessId, p);
        }
        else if (replacementPolicy == "LRU")
        {
            if (allocationPolicy == "Global")
            {
                lrulist.remove(process->PageTable[p]);
                lrulist.push_back(process->PageTable[p]);
            }
            else
            {
                process->lallocatedFrames.remove(process->PageTable[p]);
                process->lallocatedFrames.push_back(process->PageTable[p]);
            }
        }
    }
    void printPhysicalMemory()
    {
        cout << "\nPhysical Memory  - " << endl;
        for (auto it = PhysicalMemory.begin(); it != PhysicalMemory.end(); ++it)
        {
            std::cout << "f: " << it->first << ", pid: " << it->second.first << " p: " << it->second.second << std::endl;
        }
    }

    void printPageTables()
    {
        for (Process &p : ProcessList)
        {
            cout << "\n ProcessID - " << p.getProcessNumber() << endl;
            for (auto i : p.PageTable)
            {
                cout << "p: " << i.first << " f: " << i.second << endl;
            }
        }
    }

    void printFaults()
    {
        cout << "\n Global Faults: " << globalPageFault << endl;
        for (Process &p : ProcessList)
        {
            cout << "ProcessID: " << p.getProcessNumber() << " Local faults: " << p.getLocal() << endl;
        }
    }
};

string toBinaryString(uint64_t num)
{
    return bitset<64>(num).to_string();
}

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        cout << "Please enter correct number of arguments" << endl;
        return 1;
    }
    srand(time(0));
    pageSize = stoi(argv[1]);
    noOfFrames = stoi(argv[2]);
    noOfFrames_per_p = noOfFrames / 4;

    // noOfFrames_per_p=25;  //comment out as per need
    replacementPolicy = argv[3];
    allocationPolicy = argv[4];

    string filename = argv[5];
    if (noOfFrames % 4 != 0)
        k = noOfFrames % 4;
    ifstream file(filename);

    string str;

    MemoryManager mm;

    while (getline(file, str))
    {
        if (!str.empty())
        {
            for (char &ch : str)
            {
                if (ch == ',')
                {
                    ch = ' ';
                }
            }

            istringstream iss(str);

            int processId;
            iss >> processId;

            uint64_t logicalAddress;
            iss >> logicalAddress;

            if (replacementPolicy != "OPTIMAL")
            {
                mm.checkPageTable(processId, logicalAddress);
                // mm.printPhysicalMemory();
                // mm.printPageTables();
                // cout << endl;
            }
            else
            {
                // for optimal replacement policy
                info.push_back(make_pair(processId, logicalAddress));
            }
        }
    }
    file.close();

    if (replacementPolicy == "OPTIMAL")
    {
        // logic for memory allocation in case of optimal replacement policy

        for (int i = 0; i < info.size(); i++)
        {
            auto p = info[i];
            // cout << i << endl;
            CurrIdx = i; // CurrIdx is the global variable to store current iteration which is needed in replacement
            mm.checkPageTable(p.first, p.second);
            // mm.printPhysicalMemory();
            // mm.printPageTables();
            // cout << endl;
        }
    }

    // mm.printPhysicalMemory();
    // mm.printPageTables();
    mm.printFaults();

    return 0;
}
