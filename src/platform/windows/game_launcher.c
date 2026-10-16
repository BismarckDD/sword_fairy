#ifndef UNICODE
#define UNICODE 1
#endif
#ifndef _UNICODE
#define _UNICODE 1
#endif
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static int file_exists(const wchar_t *path) {
    DWORD attributes=GetFileAttributesW(path);
    return attributes!=INVALID_FILE_ATTRIBUTES && !(attributes&FILE_ATTRIBUTE_DIRECTORY);
}
static int game_root(wchar_t *root,size_t capacity,wchar_t *engine,size_t engine_capacity) {
    DWORD length=GetModuleFileNameW(NULL,root,(DWORD)capacity);
    if(!length || length>=capacity)return 0;
    wchar_t *slash=wcsrchr(root,L'\\');if(!slash)return 0;*slash=0;
    for(unsigned depth=0;depth<5;depth++) {
        wchar_t snapshot[4096],main_image[4096];
        int a=swprintf(snapshot,4096,L"%ls\\save_tool\\memory.bin",root);
        int b=swprintf(engine,engine_capacity,L"%ls\\main_decompiled\\build\\jx_native.exe",root);
        int c=swprintf(main_image,4096,L"%ls\\MAIN.EXE",root);
        if(a<0 || b<0 || c<0)return 0;
        if(file_exists(snapshot) && file_exists(engine) && file_exists(main_image))return 1;
        slash=wcsrchr(root,L'\\');if(!slash)return 0;*slash=0;
    }
    return 0;
}
typedef struct {DWORD process;int visible;HWND handle;} WindowCheck;
static BOOL CALLBACK find_game_window(HWND window,LPARAM parameter) {
    WindowCheck *check=(WindowCheck *)parameter;DWORD process=0;
    GetWindowThreadProcessId(window,&process);
    if(process==check->process && IsWindowVisible(window)){check->visible=1;check->handle=window;}
    return TRUE;
}
static int launch_error(int probe,const wchar_t *message) {
    if(!probe)MessageBoxW(NULL,message,L"剑侠情缘",MB_OK|MB_ICONERROR);
    return 1;
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,LPWSTR arguments,int show) {
    (void)instance;(void)previous;(void)show;
    int audio_probe=wcscmp(arguments,L"--verify-audio")==0;
    int production_probe=wcscmp(arguments,L"--verify-production")==0;
    int probe=production_probe || audio_probe || wcscmp(arguments,L"--verify-window")==0;
    if(*arguments && !probe)return launch_error(0,L"请直接双击启动游戏。");
    wchar_t root[4096],engine[4096],command[8192];
    if(!game_root(root,4096,engine,4096))return launch_error(probe,L"游戏文件不完整。请将启动程序保留在游戏文件夹中。");
    if(swprintf(command,8192,L"\"%ls\" 2 %ls",engine,probe&&!production_probe?(audio_probe?L"--game-window-probe":L"--original-entry-window-probe"):L"--game")<0)
        return launch_error(probe,L"游戏目录路径过长，请换到较短的目录。");
    STARTUPINFOW startup={0};startup.cb=sizeof startup;
    PROCESS_INFORMATION process={0};
    if(probe){
        startup.dwFlags=STARTF_USESTDHANDLES;
        startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
        startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
        startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    }
    if(!CreateProcessW(engine,command,NULL,NULL,probe?TRUE:FALSE,CREATE_NO_WINDOW,NULL,root,&startup,&process))
        return launch_error(probe,L"游戏暂时无法启动，请确认文件已完整解压。");
    CloseHandle(process.hThread);
    if(!probe){
        DWORD exit_code=1;
        if(WaitForSingleObject(process.hProcess,INFINITE)==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&exit_code);
        CloseHandle(process.hProcess);
        return exit_code?launch_error(0,L"游戏未能正常启动或退出，请检查游戏文件是否完整。"):0;
    }
    WindowCheck window={process.dwProcessId,0,NULL};
    for(unsigned i=0;i<100 && !window.visible;i++) {
        EnumWindows(find_game_window,(LPARAM)&window);
        if(WaitForSingleObject(process.hProcess,50)==WAIT_OBJECT_0)break;
    }
    int console_attached=AttachConsole(process.dwProcessId)!=0;
    DWORD console_error=console_attached?0:GetLastError();
    HWND console_window=console_attached?GetConsoleWindow():NULL;
    int visible_console=console_window && IsWindowVisible(console_window);
    if(console_attached)FreeConsole();
    int input_ok=1;
    if(production_probe && window.visible){
        for(unsigned i=0;i<40;i++){
            if(WaitForSingleObject(process.hProcess,250)==WAIT_OBJECT_0)break;
            LPARAM down=(LPARAM)(1u|(0x1cu<<16)),up=(LPARAM)(1u|(0x1cu<<16)|(3u<<30));
            if(!PostMessageW(window.handle,WM_KEYDOWN,VK_RETURN,down) || !PostMessageW(window.handle,WM_KEYUP,VK_RETURN,up)){input_ok=0;break;}
        }
        if(WaitForSingleObject(process.hProcess,0)!=WAIT_OBJECT_0 && !PostMessageW(window.handle,WM_CLOSE,0,0))input_ok=0;
    }
    DWORD waited=WaitForSingleObject(process.hProcess,300000),exit_code=1;
    if(waited==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&exit_code);
    else TerminateProcess(process.hProcess,3);
    CloseHandle(process.hProcess);
    wchar_t report_path[4096];
    if(swprintf(report_path,4096,L"%ls\\main_decompiled\\analysis\\native_launcher%ls_verification.json",root,production_probe?L"_production":audio_probe?L"_audio":L"")<0)return 1;
    FILE *report=_wfopen(report_path,L"wb");if(!report)return 1;
    int console_checked=console_attached || console_error==ERROR_INVALID_HANDLE;
    int passed=waited==WAIT_OBJECT_0 && exit_code==0 && window.visible && console_checked && !visible_console && input_ok;
    fprintf(report,"{\"status\":\"%s\",\"entry_mode\":\"%s\",\"visible_game_window\":%s,\"console_attached\":%s,\"visible_console_window\":%s,\"console_error\":%lu,\"child_exit_status\":%lu,\"creation_flags\":134217728,\"scope\":\"Actual GUI launcher root resolution, visible native game window, no visible console and bounded keyboard/close or window/audio fixture. An attachable hidden console is allowed; full interactive campaign is separate.\"}\n",
        passed?"PASS":"FAIL",production_probe?"production":audio_probe?"audio_fixture":"original_entry_fixture",window.visible?"true":"false",console_attached?"true":"false",visible_console?"true":"false",(unsigned long)console_error,(unsigned long)exit_code);
    fclose(report);return passed?0:1;
}
