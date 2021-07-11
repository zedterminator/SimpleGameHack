#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

using namespace std;

HANDLE hProcSnap = NULL;
PROCESSENTRY32 procEntry32;

HANDLE hModuleSnap = NULL;
MODULEENTRY32 modEntry32;

DWORD pID = NULL;

HANDLE hProc = NULL;

bool attachProc(char* procName)
{
    procEntry32.dwSize = sizeof(PROCESSENTRY32);
    hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Khong the snap cac process" << endl;
        return false;
    }

    while (Process32Next(hProcSnap, &procEntry32))
    {
        if (!strcmp(procName, procEntry32.szExeFile))
        {
            cout << "Tim thay process " << procEntry32.szExeFile << " voi pID la " << procEntry32.th32ProcessID << endl;
            pID = procEntry32.th32ProcessID;
            hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procEntry32.th32ProcessID);

            if (hProc == NULL)
            {
                cout << "Khong the tao handle cua process, exit." << endl;
                return false;
            }
            CloseHandle(hProcSnap);
            return true;
        }
    }

    cout << "Khong the tim thay process " << procName << " trong list process, vui long thu lai sau." << endl;
    CloseHandle(hProcSnap);
    return false;
}

DWORD getModule(LPSTR moduleName)
{
    hModuleSnap = INVALID_HANDLE_VALUE;
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Khong the snapshot module list cua process " << pID << endl;
        CloseHandle(hModuleSnap);
        return false;
    }

    modEntry32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hModuleSnap, &modEntry32))
    {
        if (!strcmp(moduleName, modEntry32.szModule))
        {
            cout << "Tim thay module " << modEntry32.szModule << " voi base address la: " << hex << (DWORD)modEntry32.modBaseAddr << endl;

            CloseHandle(hModuleSnap);
            return (DWORD)modEntry32.modBaseAddr;
        }
    }

    while (Module32Next(hModuleSnap, &modEntry32))
    {
        if (!strcmp(moduleName, modEntry32.szModule))
        {
            cout << "Tim thay module " << modEntry32.szModule << " voi base address la: " << hex << (DWORD)modEntry32.modBaseAddr << endl;
            CloseHandle(hModuleSnap);
            return (DWORD)modEntry32.modBaseAddr;
        }
    }

    cout << "Khong the tim thay module " << moduleName << " trong process " << pID << endl;
    CloseHandle(hModuleSnap);
    return false;
}

DWORD getPointerAddress(DWORD gameBaseAddress, DWORD address, vector<DWORD> offsets)
{
    DWORD offset_null = NULL;
    ReadProcessMemory(hProc, (LPVOID*)(gameBaseAddress + address), &offset_null, sizeof(offset_null), 0);
    DWORD pointerAddress = offset_null;
    for (int i = 0; i < offsets.size() - 1; i++)
    {
        ReadProcessMemory(hProc, (LPVOID*)(pointerAddress + offsets.at(i)), &pointerAddress, sizeof(pointerAddress), 0);
    }
    return pointerAddress += offsets.at(offsets.size() - 1);
}

template <class dataType>
void wpm(dataType valToWrite, DWORD addressToWrite)
{
    WriteProcessMemory(hProc, (PVOID)addressToWrite, &valToWrite, sizeof(dataType), 0);
}

template <class dataType>
void rpm(dataType valToRead, DWORD addressToRead)
{
    dataType rpmBuffer;
    ReadProcessMemory(hProc, (PVOID)addressToRead, &rpmBuffer, sizeof(dataType), 0);
    return rpmBuffer;
}


int main()
{
    DWORD moduleBaseAddr = NULL;
    DWORD sunAddress = 0x00329670;
    DWORD realAddress = NULL;

    int val = 9999; // số lượng mặt trời muốn ghi vào bộ nhớ

    vector<DWORD> offsets = { 0x320, 0x18, 0x0, 0x8, 0x5578 };

    bool check = attachProc((char*)"popcapgame1.exe");
    if (check)
    {
        cout << "Thanh cong attach vao process " << pID << endl;
        moduleBaseAddr = getModule((LPSTR)"popcapgame1.exe");
        if (moduleBaseAddr == NULL)
        {
            cout << "Khong tim duoc module base address" << endl;
            return 0;
        }
        cout << "Base address: " << hex << moduleBaseAddr << endl;

        realAddress = getPointerAddress(moduleBaseAddr, sunAddress, offsets);

        if (realAddress == NULL)
        {
            cout << "Khong tim duoc realaddress cua " << hex << sunAddress << endl;
            return 0;
        }
        cout << "Dia chi sunaddress: " << hex << realAddress << endl;
        cout << "Nhap so luong mat troi: ";
        cin >> val;
        while (true)
        {
            wpm<int>(val, realAddress);
            cout << "Da thuc hien ghi " << val << " SUN vao bo nho" << endl;
            Sleep(1000);
        }
    }
    else {
        cout << "Khong the attach process, vui long thu lai";
        return 0;
    }

}