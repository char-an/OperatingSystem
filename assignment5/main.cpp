#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdint>

using namespace std;

int globalPageFault;

class Process
{
public:
    int ProcessId;
    unordered_map<uint64_t, uint64_t> PageTable;
    int localPageFault;

    Process(int ProcessId)
    {
        this->ProcessId = ProcessId;
    }

    void Map(uint64_t logicalAddress)
    {
        PageTable.insert({logicalAddress, 0});
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

    void allocateMemory(int ProcessId, uint64_t logicalAddress)
    {
        Process *p = getProcessByProcessNumber(ProcessId);

        if (p == nullptr)
        {
            Process newProcess(ProcessId);
            ProcessList.push_back(newProcess);

            p = &ProcessList.back();
        }

        p->Map(logicalAddress);
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
};

class PhysicalMemory
{
};

string toBinaryString(uint64_t num){
    string rep = "";
    for(int i=63;i>=0;i++){
        int bit = num &(1 << i);
        if(bit==0){
            rep += '0';
        }
        else{
            rep+= '1';
        }
    }
    return rep;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Please enter correct number of arguments" << endl;
        return 1;
    }

    string filename = argv[1];
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

            //cout << "ProcessId is: " << processId << " and Logical address is: " << logicalAddress << endl;
            mm.allocateMemory(processId, logicalAddress);
        }
    }

    file.close();
    return 0;
}