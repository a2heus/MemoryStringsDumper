#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <thread>
#include <chrono>
#include <random>

using namespace std;

struct ProcessInfo {
    DWORD pid;
    std::string exeName;
};

bool g_stopTitleThread = false;

// Returns a random alphanumeric string of given length
string randomString(size_t length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> dist(0, max_index - 1);
    string str;
    for (size_t i = 0; i < length; i++) {
        str += charset[dist(generator)];
    }
    return str;
}

// Thread function that updates the console title every 0.1 seconds
void updateTitleThread() {
    while (!g_stopTitleThread) {
        string title = randomString(32) + " made by hash_014";
        SetConsoleTitleA(title.c_str());
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

string toLower(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

vector<ProcessInfo> getProcessList() {
    vector<ProcessInfo> processes;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return processes;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32FirstW(hSnapshot, &pe)) {
        wstring_convert<codecvt_utf8<wchar_t>> converter;
        do {
            DWORD sessionId = 0;
            if (!ProcessIdToSessionId(pe.th32ProcessID, &sessionId) || sessionId == 0)
                continue;
            string processName = converter.to_bytes(pe.szExeFile);
            string lowerName = toLower(processName);
            if (lowerName == "system" || lowerName == "idle")
                continue;
            ProcessInfo p;
            p.pid = pe.th32ProcessID;
            p.exeName = processName;
            processes.push_back(p);
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return processes;
}

void displayProcessPage(const vector<ProcessInfo>& processes, int currentPage, int selectedIndex, int pageSize) {
    system("cls");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int totalPages = (processes.size() + pageSize - 1) / pageSize;
    cout << "Page " << (currentPage + 1) << "/" << totalPages << "\n\n";
    int startIndex = currentPage * pageSize;
    int endIndex = min(startIndex + pageSize, (int)processes.size());
    for (int i = startIndex; i < endIndex; i++) {
        if (i == currentPage * pageSize + selectedIndex) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            cout << "> ";
        }
        else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << "  ";
        }
        cout << processes[i].exeName << " (PID: " << processes[i].pid << ")\n";
    }
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << "\nNavigation:\n";
    cout << "  Up/Down arrows   : move selection within page\n";
    cout << "  Left/Right arrows: change page\n";
    cout << "  Enter            : confirm selection\n";
}

string extractStringsFromBuffer(const char* buffer, SIZE_T size) {
    string result;
    string current;
    for (SIZE_T i = 0; i < size; i++) {
        char c = buffer[i];
        if (isprint(static_cast<unsigned char>(c))) {
            current.push_back(c);
        }
        else {
            if (current.length() >= 4)
                result += current + "\n";
            current.clear();
        }
    }
    if (current.length() >= 4)
        result += current + "\n";
    return result;
}

int main() {
    // Start the title updating thread
    thread titleThread(updateTitleThread);

    vector<ProcessInfo> processes = getProcessList();
    if (processes.empty()) {
        cout << "No user processes found.\n";
        system("pause");
        g_stopTitleThread = true;
        titleThread.join();
        return 1;
    }
    const int pageSize = 10;
    int totalPages = (processes.size() + pageSize - 1) / pageSize;
    int currentPage = 0;
    int selectedIndex = 0;
    displayProcessPage(processes, currentPage, selectedIndex, pageSize);
    while (true) {
        int ch = _getch();
        if (ch == 0 || ch == 224) {
            int arrow = _getch();
            switch (arrow) {
            case 72: // up arrow
                if (selectedIndex > 0)
                    selectedIndex--;
                else if (currentPage > 0) {
                    currentPage--;
                    int startIndex = currentPage * pageSize;
                    int endIndex = min(startIndex + pageSize, (int)processes.size());
                    selectedIndex = (endIndex - startIndex) - 1;
                }
                break;
            case 80: { // down arrow
                int startIndex = currentPage * pageSize;
                int endIndex = min(startIndex + pageSize, (int)processes.size());
                if (selectedIndex < (endIndex - startIndex) - 1)
                    selectedIndex++;
                else if (currentPage < totalPages - 1) {
                    currentPage++;
                    selectedIndex = 0;
                }
                break;
            }
            case 75: // left arrow
                if (currentPage > 0) {
                    currentPage--;
                    selectedIndex = 0;
                }
                break;
            case 77: // right arrow
                if (currentPage < totalPages - 1) {
                    currentPage++;
                    selectedIndex = 0;
                }
                break;
            default:
                break;
            }
            displayProcessPage(processes, currentPage, selectedIndex, pageSize);
        }
        else if (ch == 13) { // enter key
            break;
        }
    }
    ProcessInfo target = processes[currentPage * pageSize + selectedIndex];
    cout << "\nSelected process: " << target.exeName << " (PID: " << target.pid << ")\n";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, target.pid);
    if (!hProcess) {
        cout << "Failed to open process. Check your privileges.\n";
        system("pause");
        g_stopTitleThread = true;
        titleThread.join();
        return 1;
    }
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    LPVOID addr = sysInfo.lpMinimumApplicationAddress;
    string allStrings;
    while (addr < sysInfo.lpMaximumApplicationAddress) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQueryEx(hProcess, addr, &mbi, sizeof(mbi)) == 0)
            break;
        if (mbi.State == MEM_COMMIT &&
            (mbi.Protect & PAGE_READONLY || mbi.Protect & PAGE_READWRITE ||
                mbi.Protect & PAGE_EXECUTE_READ || mbi.Protect & PAGE_EXECUTE_READWRITE)) {
            char* buffer = new char[mbi.RegionSize];
            SIZE_T bytesRead;
            if (ReadProcessMemory(hProcess, addr, buffer, mbi.RegionSize, &bytesRead))
                allStrings += extractStringsFromBuffer(buffer, bytesRead);
            delete[] buffer;
        }
        addr = (LPVOID)((char*)mbi.BaseAddress + mbi.RegionSize);
    }
    CloseHandle(hProcess);
    time_t t = time(nullptr);
    tm tmStruct;
    localtime_s(&tmStruct, &t);
    ostringstream oss;
    oss << put_time(&tmStruct, "%Y%m%d%H%M%S") << ".txt";
    string filename = oss.str();
    ofstream outFile(filename);
    if (!outFile) {
        cout << "Error opening output file.\n";
        system("pause");
        g_stopTitleThread = true;
        titleThread.join();
        return 1;
    }
    outFile << allStrings;
    outFile.close();
    cout << "\nMemory dump complete. Strings saved in: " << filename << "\n";
    cout << "\nPress any key to exit.\n";
    _getch();
    g_stopTitleThread = true;
    titleThread.join();
    return 0;
}
