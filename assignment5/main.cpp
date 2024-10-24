#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
using namespace std;

class Process
{
public:
    int ProcessId;
    unordered_map<unsigned long long int, int> PageTable;

    Process(int ProcessId)
    {
        this->ProcessId = ProcessId;
    }

    void Map(unsigned long long int logicalAddress)
    {
        PageTable.insert({logicalAddress, 0});
    }

    int getProcessNumber()
    {
        return this->ProcessId;
    }
};

class MemoryManager
{
public:
    vector<Process> ProcessList;

    void allocateMemory(int ProcessId, unsigned long long int logicalAddress)
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

            unsigned long long int logicalAddress;
            iss >> logicalAddress;

            // cout << "ProcessId is: " << processId << " and Logical address is: " << logicalAddress << endl;
            mm.allocateMemory(processId, logicalAddress);
        }
    }

    file.close();
    return 0;
}
