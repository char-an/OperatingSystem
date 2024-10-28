#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include <bitset>

using namespace std;

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

    void checkPageTable(int ProcessId,uint64_t logicalAddress){
        Process *process = getProcessByProcessNumber(ProcessId);

        if (process == nullptr)
        {
            Process newProcess(ProcessId);
            ProcessList.push_back(newProcess);

            process = &ProcessList.back();
        }

        string binary = toBinaryString(logicalAddress);
        cout << "Logical address : "<< binary << endl;
        // uint64_t p = stoi(binary.substr(0,52), nullptr, 2); //change depending on page size
        // uint64_t d = stoi(binary.substr(52,12), nullptr,2);  //change depending on page size
        uint64_t p = stoull(binary.substr(0, 52), nullptr, 2);
        uint64_t d = stoull(binary.substr(52, 12), nullptr, 2);

        cout << "Page number : "<< p << endl;
        cout << "Offset : "<< d << endl;


        auto it = process->PageTable.find(p);
        if(it!=process->PageTable.end()){
            cout << "Found!" << endl;
        }else{
            cout << "pagefault" << endl;
            process->PageTable[p] = 0; //temp
        }
    }

};

class PhysicalMemory
{
};

string toBinaryString(uint64_t num){
    return bitset<64>(num).to_string();
}

//incorrect binary rep

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

            cout << "\n \n ProcessId is: " << processId << " and Logical address is: " << logicalAddress << endl;
            // mm.allocateMemory(processId, logicalAddress);
            mm.checkPageTable(processId, logicalAddress);
            
        }
    }

    file.close();
    return 0;
}