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
string replacementPolicy;
string allocationPolicy;

int globalPageFault;
string toBinaryString(uint64_t num);

queue<int> fifoQueue;
list<int> lrulist;
// vector for storing file info, used only in case of Optimal replacement policy
vector<pair<int, uint64_t>> info;
vector<int> frameVec;
int CurrIdx;

class Process
{
public:
    int ProcessId;
    unordered_map<uint64_t, uint64_t> PageTable;
    int localPageFault;

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
        return this->ProcessId;
    }

    void incrementLocal()
    {
        this->localPageFault++;
    }

    int getLocal()
    {
        return this->localPageFault;
    }
};

class MemoryManager
{
public:
    vector<Process> ProcessList;
    map<int, pair<int, uint64_t>> PhysicalMemory; // key:frame , value:pid,page no.

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
        {
            if (p.getProcessNumber() == ProcessNumber)
            {
                return &p;
            }
        }
        return nullptr;
    }

    // check if free frames present
    void checkFrames(int ProcessId, uint64_t p)
    {
        Process *process = getProcessByProcessNumber(ProcessId);

        int f = -1;

        int length = PhysicalMemory.size();
        // cout << "size of physical memory  " << length << endl;
        if (length < noOfFrames)
        {               // free
            f = length; // frame number
            // cout << "process id : " << ProcessId << " frame no. : " << f << endl;
            PhysicalMemory[f] = make_pair(ProcessId, p); // pid and page no.
            allocateMemory(ProcessId, p, f);

            if (replacementPolicy == "FIFO")
            {
                fifoQueue.push(f);
            }
            else if (replacementPolicy == "LRU")
            {
                lrulist.push_back(f);
            }
            else if (replacementPolicy == "OPTIMAL")
            {
                frameVec.push_back(f);
            }
        }
        else if (length == noOfFrames)
        { // need to replace
            if (replacementPolicy == "FIFO")
            {
                FIFO(ProcessId, p);
            }
            else if (replacementPolicy == "LRU")
            {
                LRU(ProcessId, p);
            }
            else if (replacementPolicy == "RANDOM")
            {
                RANDOM(ProcessId, p);
            }
            else if (replacementPolicy == "OPTIMAL")
            {
                OPIMAL(ProcessId, p);
            }
        }
        else
        {
            cout << "ERROR!!" << endl;
        }
    }

    int frameToBeRemoved()
    {
        int resultantFrame = -1;
        int maxDist = -1;

        for (int frame : frameVec)
        {
            auto it = PhysicalMemory.find(frame); // finding virtual logical address corresponding to frame from PhysicalMemory map
            int Dist = 0;
            if (it != PhysicalMemory.end())
            {
                for (int i = CurrIdx; i < info.size(); i++)
                {
                    // checking when same logical address will be used
                    auto iter = info[i];
                    string binary = toBinaryString(iter.second);
                    int dSize = log2(pageSize);
                    int pSize = 64 - dSize;
                    uint64_t p = stoull(binary.substr(0, pSize), nullptr, 2);
                    // cout << "page Number going to be used : " << p << " after " << Dist << endl;
                    // cout << "page Number of frame in memory : " << it->second.second << endl;
                    // cout << "corresponding frame : " << frame << endl;
                    Dist++;
                    if(p == it->second.second){
                        break;
                    }
                }
            }

            if(Dist > maxDist){
                maxDist = Dist;
                resultantFrame = frame;
            }
        }
        return resultantFrame;
    }

    void OPIMAL(int ProcessId, uint64_t p)
    {
        // cout << "OPTIMAL" << endl;
        int f = frameToBeRemoved();
        if(f == -1){
            cout << "Error , f == -1" << endl;
            return ;
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

    void LRU(int ProcessId, uint64_t p)
    {
        int f = lrulist.front();
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
        lrulist.pop_front(); // remove head of the linked list
        PhysicalMemory[f] = make_pair(ProcessId, p);
        allocateMemory(ProcessId, p, f);
        lrulist.push_back(f);
    }

    void FIFO(int ProcessId, uint64_t p)
    {
        int f = fifoQueue.front(); // frame to replace
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
        fifoQueue.pop();
        PhysicalMemory[f] = make_pair(ProcessId, p);
        allocateMemory(ProcessId, p, f);
        fifoQueue.push(f);
    }

    void RANDOM(int ProcessId, uint64_t p)
    {
        int f = rand() % noOfFrames; // generates a random frame index within range of no.of frames
        // cout << "replacement - process id : " << ProcessId << " frame no. : " << f << endl;
        auto it = PhysicalMemory.find(f);
        if (it != PhysicalMemory.end())
        {
            deletePageTableMapping(it->second.first, it->second.second);
            PhysicalMemory.erase(f);
        }
        PhysicalMemory[f] = make_pair(ProcessId, p);
        allocateMemory(ProcessId, p, f);
    }

    void deletePageTableMapping(int ProcessId, uint64_t p)
    {
        Process *process = getProcessByProcessNumber(ProcessId);
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
        int pSize = 64 - dSize;                                       // since 64bit system
        uint64_t p = stoull(binary.substr(0, pSize), nullptr, 2);     // change depending on page size
        uint64_t d = stoull(binary.substr(pSize, dSize), nullptr, 2); // change depending on page size

        // cout << "Page number : " << p << endl;
        // cout << "Offset : " << d << endl;

        auto it = process->PageTable.find(p);
        if (it != process->PageTable.end())
        { // found
            // cout << "Found!" << endl;
            if (replacementPolicy == "LRU")
            {
                // whenever frame is accessed from pageTable, we will update linked list
                lrulist.remove(process->PageTable[p]);
                lrulist.push_back(process->PageTable[p]);
            }
        }
        else
        { // page fault
            // cout << "pagefault" << endl;
            globalPageFault++;
            process->incrementLocal();
            // allocation
            checkFrames(ProcessId, p);
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

    pageSize = stoi(argv[1]);
    noOfFrames = stoi(argv[2]);
    replacementPolicy = argv[3];
    allocationPolicy = argv[4];

    string filename = argv[5];
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

    mm.printPhysicalMemory();
    mm.printPageTables();
    mm.printFaults();

    return 0;
}