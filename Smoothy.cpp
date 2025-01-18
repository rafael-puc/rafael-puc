// BUDOKKAN SMOOTHY V01-18-2025
#include <windows.h>
#include <tlhelp32.h>
#include <dwmapi.h>
#include <iostream>
#include <psapi.h>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>

// ENVIROMENT
#pragma comment(lib, "dwmapi.lib")
#define ProcessBreakOnTermination ((PROCESS_INFORMATION_CLASS)0x1D)
// PROCESS QUERYING
extern "C" __declspec(dllimport) BOOL WINAPI IsProcessCritical(HANDLE hProcess, PBOOL Critical);
typedef NTSTATUS(NTAPI* NtSetTimerResolution_pfn)(IN ULONG DesiredResolution,IN BOOLEAN SetResolution,OUT PULONG CurrentResolution);NtSetTimerResolution_pfn NtSetTimerResolution=nullptr;ULONG gDesiredResolution=1;bool gNoConsole=false;typedef BOOL(WINAPI* SetBoostProcessesFunc)(HWND,DWORD,HANDLE*);
// CHAR/STRING CONVERSION
std::wstring ConvertToWideString(const char* str){int len=MultiByteToWideChar(CP_ACP,0,str,-1,NULL,0);if(len==0)return L"";wchar_t* wideStr=new wchar_t[len];MultiByteToWideChar(CP_ACP,0,str,-1,wideStr,len);std::wstring result(wideStr);delete[] wideStr;return result;}

// FUNCTION: FETCH/UPDATE CURRENT TIMER RESOLUTION
void SetResolution(ULONG desiredResolution){ULONG currentRes=0;NTSTATUS status=NtSetTimerResolution(desiredResolution, TRUE, &currentRes);
if (status != 0){if (!gNoConsole) std::cerr << "Unavailable..." << status << std::endl;return;} if (!gNoConsole) std::cout << "\nMMCSS Attatched to DWM. \nTimer Resolution ON.\n\n" << std::endl;
while (true){auto start=std::chrono::high_resolution_clock::now();Sleep(1);auto end=std::chrono::high_resolution_clock::now();double elapsed=std::chrono::duration<double, std::milli>(end - start).count();double delta=elapsed - 1.0;
if (!gNoConsole) std::cout << "Timer Applied: " << std::fixed << std::setprecision(4) << desiredResolution / 10000.0 << "ms / Timer: " << elapsed << " ms / Delta: " << delta << "ms" << std::endl;std::this_thread::sleep_for(std::chrono::seconds(1));}}

// FUNCTION: DWM_TRANSITIONS_OFF/DWM_COMPOSITION_OFF/DPISCALE_OFF/MMCSS_ENGAGE
void OptimizeDWM() {DwmEnableComposition(DWM_EC_DISABLECOMPOSITION); HWND hwnd = GetDesktopWindow();if (hwnd) {BOOL disableTransitions = TRUE; DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disableTransitions, sizeof(disableTransitions));} DwmEnableMMCSS(TRUE); HMODULE hUser32 = LoadLibraryW(L"user32.dll");
if (hUser32) {typedef BOOL(WINAPI* SetProcessDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT);auto SetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContext_t)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
if (SetProcessDpiAwarenessContext) {std::vector<std::wstring> targetProcesses={L"League of Legends.exe",L"Dwm.exe", L"FortniteClient-Win64-Shipping.exe",L"Valorant-Win64-Shipping.exe"}; HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
if (hSnapshot != INVALID_HANDLE_VALUE) { PROCESSENTRY32 pe32; pe32.dwSize = sizeof(PROCESSENTRY32);if (Process32First(hSnapshot, &pe32)) {do { std::wstring exeName = ConvertToWideString(pe32.szExeFile);
if (std::find(targetProcesses.begin(), targetProcesses.end(), exeName) != targetProcesses.end()) {HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe32.th32ProcessID);if (hProcess) {SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE); CloseHandle(hProcess);}}}
while (Process32Next(hSnapshot, &pe32));} CloseHandle(hSnapshot);}} FreeLibrary(hUser32);} std::this_thread::sleep_for(std::chrono::seconds(50));}

// FUNCTION: DISABLE FLIP 3D
void Flip3DPolicy() {const std::vector<std::wstring> targetProcesses = {L"League of Legends.exe",L"Dwm.exe",L"FortniteClient-Win64-Shipping.exe",L"Valorant-Win64-Shipping.exe"};
while (true){HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);if (hSnapshot==INVALID_HANDLE_VALUE){continue;} PROCESSENTRY32 pe32; pe32.dwSize=sizeof(PROCESSENTRY32);if (Process32First(hSnapshot, &pe32)){do {std::wstring exeName = ConvertToWideString(pe32.szExeFile);
if (std::find(targetProcesses.begin(), targetProcesses.end(), exeName) != targetProcesses.end()) {HWND hwnd = GetTopWindow(NULL);while (hwnd) {DWORD windowProcessId = 0; GetWindowThreadProcessId(hwnd, &windowProcessId);
if (windowProcessId == pe32.th32ProcessID) {DWORD dwmPolicy = DWMFLIP3D_EXCLUDEABOVE; DwmSetWindowAttribute(hwnd, DWMWA_FLIP3D_POLICY, &dwmPolicy, sizeof(dwmPolicy)); break;} hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);}}}
while (Process32Next(hSnapshot, &pe32));} CloseHandle(hSnapshot); std::this_thread::sleep_for(std::chrono::seconds(50));}}

// FUNCTION: REALTIME / CRITICAL LABEL / ALLOCATED MEMORY
void PriorityCritWorkset(const std::wstring& processName1, const std::wstring& processName2, SIZE_T minSize, SIZE_T maxSize, DWORD priorityClass){HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(hSnapshot==INVALID_HANDLE_VALUE)return;PROCESSENTRY32 pe32;pe32.dwSize=sizeof(PROCESSENTRY32);
if(Process32First(hSnapshot,&pe32)){do{std::wstring exeName=ConvertToWideString(pe32.szExeFile);if(exeName==processName1||exeName==processName2){HANDLE hProcess=OpenProcess(PROCESS_SET_INFORMATION|PROCESS_QUERY_INFORMATION|PROCESS_SET_QUOTA,FALSE,pe32.th32ProcessID);if(hProcess){BOOL isCritical=FALSE;
if(IsProcessCritical(hProcess,&isCritical)&&!isCritical){HMODULE hNtdll=GetModuleHandleW(L"ntdll.dll");if(hNtdll){typedef NTSTATUS(WINAPI* NtSetInformationProcess_t)(HANDLE,PROCESS_INFORMATION_CLASS,PVOID,ULONG); auto NtSetInformationProcess=(NtSetInformationProcess_t)GetProcAddress(hNtdll,"NtSetInformationProcess");
if(NtSetInformationProcess){struct{BOOL IsCritical;HANDLE BreakOnTermination;}criticalInfo={TRUE,NULL};NtSetInformationProcess(hProcess,ProcessBreakOnTermination,&criticalInfo,sizeof(criticalInfo));}}}SetProcessWorkingSetSize(hProcess,minSize,maxSize);SetPriorityClass(hProcess,priorityClass);CloseHandle(hProcess);}}}
while(Process32Next(hSnapshot,&pe32));}CloseHandle(hSnapshot);}void PriorityCritSet(){while(true){PriorityCritWorkset(L"Dwm.exe",L"Csrss.exe",1ULL*1024*1024*1024,3ULL*1024*1024*1024,REALTIME_PRIORITY_CLASS);std::this_thread::sleep_for(std::chrono::seconds(50));}}

// FUNCTION: SET FOREGROUND BOOST
void ForegroundBoost(){HMODULE hUser32=LoadLibraryW(L"user32.dll");if(!hUser32)return;auto SetBoostProcesses=(SetBoostProcessesFunc)GetProcAddress(hUser32,"SetAdditionalForegroundBoostProcesses");
if(!SetBoostProcesses){FreeLibrary(hUser32);return;}const std::vector<std::wstring> targetProcesses={L"League of Legends.exe",L"Dwm.exe",L"FortniteClient-Win64-Shipping.exe",L"Valorant-Win64-Shipping.exe"};while(true){HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
if(hSnapshot==INVALID_HANDLE_VALUE)return;PROCESSENTRY32 pe32;pe32.dwSize=sizeof(PROCESSENTRY32);if(Process32First(hSnapshot,&pe32)){do{std::wstring exeName=ConvertToWideString(pe32.szExeFile);if(std::find(targetProcesses.begin(),targetProcesses.end(),exeName)!=targetProcesses.end()){DWORD processId=pe32.th32ProcessID;HWND hwnd=GetTopWindow(NULL);
while(hwnd){DWORD windowProcessId=0;GetWindowThreadProcessId(hwnd,&windowProcessId);if(windowProcessId==processId){HANDLE hProcess=OpenProcess(PROCESS_SET_INFORMATION,FALSE,processId);if(hProcess){HANDLE processesToBoost[1]={hProcess};SetBoostProcesses(hwnd,1,processesToBoost);CloseHandle(hProcess);}break;}hwnd=GetNextWindow(hwnd,GW_HWNDNEXT);}}}
while(Process32Next(hSnapshot,&pe32));}CloseHandle(hSnapshot);std::this_thread::sleep_for(std::chrono::seconds(50));}FreeLibrary(hUser32);}

// MAIN
int main(int argc, char* argv[]) { for (int i = 1; i < argc; ++i) { if (std::string(argv[i]) == "--hide") { gNoConsole = true; HWND hwnd = GetConsoleWindow(); if (hwnd) ShowWindow(hwnd, SW_HIDE); } 
else if (std::string(argv[i]) == "--timer" && i + 1 < argc) { gDesiredResolution = std::stoul(argv[i + 1]); ++i; } } HMODULE hNtDll = LoadLibraryW(L"ntdll.dll"); if (!hNtDll) return 1; NtSetTimerResolution = (NtSetTimerResolution_pfn)GetProcAddress(hNtDll, "NtSetTimerResolution");
// CALL DWM_MMCSS OPTIMIZER
std::thread(OptimizeDWM).detach();
// CALL REALTIME_CRITLABEL_ALLOCMEM
std::thread(PriorityCritSet).detach();
// CALL DISABLE_FLIP3D
std::thread(Flip3DPolicy).detach();
// CALL FOREGROUND_BOOST
std::thread(ForegroundBoost).detach();
// CALL TIMER_RESOLUTION
SetResolution(gDesiredResolution);

return 0;
}
