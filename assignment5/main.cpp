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

using namespace std;

int pageSize;
int noOfFrames;
string replacementPolicy;
string allocationPolicy;

int globalPageFault;
string toBinaryString(uint64_t num);

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

    void Map(uint64_t logicalAddress)
    {
        //PageTable.insert({logicalAddress, 0});
        PageTable[logicalAddress] = 0;
    }

    int getProcessNumber()
    {
        return this->ProcessId;
    }

    void incrementLocal(){
        this->localPageFault++;
    }

    int getLocal(){
        return this->localPageFault;
    }
};

class MemoryManager
{
public:
    vector<Process> ProcessList;
    map<int,int> PhysicalMemory;

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

    //check if free frames present
    void checkFrames(int ProcessId,uint64_t p){
        Process *process = getProcessByProcessNumber(ProcessId);

        if (process == nullptr)
        {
            Process newProcess(ProcessId);
            ProcessList.push_back(newProcess);

            process = &ProcessList.back();
        }

        int frameNumber = -1;

        int length = PhysicalMemory.size();
        //cout << "size of physical memory  " << length << endl;
        if(length <= noOfFrames){ // free
            PhysicalMemory[length] = p;
            frameNumber = length;
            cout << "proces id : " << ProcessId << " frame no. : " << frameNumber << endl;
            allocateMemory(ProcessId,p,frameNumber);
        }else{                    // need to replace

        }
    }

    void checkPageTable(int ProcessId,uint64_t logicalAddress){
        Process *process = getProcessByProcessNumber(ProcessId);

        if (process == nullptr)
        {
            Process newProcess(ProcessId);
            ProcessList.push_back(newProcess);

            process = &ProcessList.back();
        }

        string binary = toBinaryString(logicalAddress);
        // cout << "Logical address : "<< binary << endl;
        int dSize = log2(pageSize);
        int pSize = 64 - dSize;
        uint64_t p = stoull(binary.substr(0, pSize), nullptr, 2);  // change depending on page size
        uint64_t d = stoull(binary.substr(pSize, dSize), nullptr, 2); //change depending on page size

        cout << "Page number : "<< p << endl;
        cout << "Offset : "<< d << endl;


        auto it = process->PageTable.find(p);
        if(it!=process->PageTable.end()){ //found
            cout << "Found!" << endl;
        }else{                            //page fault
            cout << "pagefault" << endl;
            globalPageFault++;
            process->incrementLocal();
            //allocation
            checkFrames(ProcessId,p);
        }
    }

};

string toBinaryString(uint64_t num){
    return bitset<64>(num).to_string();
}

//incorrect binary rep

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

            cout << "\n \n ProcessId is: " << processId << " and Logical address is: " << logicalAddress << endl;
            //mm.allocateMemory(processId, logicalAddress);
            mm.checkPageTable(processId, logicalAddress);
            
        }
    }

    file.close();
    return 0;
}