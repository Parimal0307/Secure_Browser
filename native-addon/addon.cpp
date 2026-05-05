#include <napi.h>
#include <windows.h>
#include <string>

// ----------------------------
// GLOBALS
// ----------------------------
HHOOK keyboardHook = NULL;
Napi::ThreadSafeFunction tsfn;

HWND appWindow = NULL;
bool focusMonitoring = false;

static bool isWinPressed = false;

// ----------------------------
// SAFE EVENT SENDER
// ----------------------------
void SendEvent(const std::string &msg)
{
    if (tsfn)
    {
        tsfn.BlockingCall(new std::string(msg),
                          [](Napi::Env env, Napi::Function jsCallback, std::string *data)
                          {
                              jsCallback.Call({Napi::String::New(env, *data)});
                              delete data;
                          });
    }
}

// ----------------------------
// KEYBOARD HOOK
// ----------------------------
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

        bool isAltPressed = (GetAsyncKeyState(VK_MENU) & 0x8000);
        bool isCtrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
        bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
       

        // ----------------------------
        // BLOCK SHORTCUTS
        // ----------------------------

        // Alt + Tab
        if (p->vkCode == VK_TAB && isAltPressed)
        {
            SendEvent("ALT_TAB_BLOCKED");
            return 1;
        }

        // Alt + Esc
        if (p->vkCode == VK_ESCAPE && isAltPressed)
        {
            SendEvent("ALT_ESC_BLOCKED");
            return 1;
        }

        // Alt + F4 (Close)
        if (p->vkCode == VK_F4 && isAltPressed)
        {
            SendEvent("ALT_F4_BLOCKED");
            return 1;
        }

        if (p->vkCode == VK_F1 && isAltPressed)
        {
            SendEvent("ALT_F1_BLOCKED");
            return 1;
        }

        if (p->vkCode == VK_F2 && isAltPressed)
        {
            SendEvent("ALT_F2_BLOCKED");
            return 1;
        }

        if (p->vkCode == VK_F3 && isAltPressed)
        {
            SendEvent("ALT_F3_BLOCKED");
            return 1;
        }

        // 1. Track Win key state first (no return yet)
        if (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN)
        {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
                isWinPressed = true;
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
                isWinPressed = false;
        }

        // 2. Handle combinations FIRST
        // Project menu
        if (p->vkCode == 'P' && isWinPressed)
        {
            SendEvent("PROJECT_MENU_BLOCKED");
            return 1;
        }

        // Win + Down (Minimize)
        if (p->vkCode == VK_DOWN && isWinPressed)
        {
            SendEvent("WIN_MINIMIZE_BLOCKED");
            return 1;
        }

        // Win + K (cast menu)
        if (p->vkCode == 'K' && isWinPressed)
        {
            SendEvent("CAST_MENU_BLOCKED");
            return 1;
        }

        // 3. Handle single key fallback
        if (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN)
        {
            SendEvent("WINDOWS_KEY_BLOCKED");
            return 1;
        }

        // Ctrl + Shift + Esc (Task Manager)
        if (p->vkCode == VK_ESCAPE && isCtrlPressed && isShiftPressed)
        {
            SendEvent("CTRL_SHIFT_ESC_BLOCKED");
            return 1;
        }

        // Ctrl + Esc
        if (p->vkCode == VK_ESCAPE && isCtrlPressed)
        {
            SendEvent("CTRL_ESC_BLOCKED");
            return 1;
        }

        // Alt + Space (System menu)
        if (p->vkCode == VK_SPACE && isAltPressed)
        {
            SendEvent("ALT_SPACE_BLOCKED");
            return 1;
        }

        // Ctrl + C
        if (p->vkCode == 'C' && isCtrlPressed)
        {
            SendEvent("COPY_BLOCKED");
            return 1;
        }

        // Ctrl + V
        if (p->vkCode == 'V' && isCtrlPressed)
        {
            SendEvent("PASTE_BLOCKED");
            return 1;
        }

        // Ctrl + X
        if (p->vkCode == 'X' && isCtrlPressed)
        {
            SendEvent("CUT_BLOCKED");
            return 1;
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// ----------------------------
// KEYBOARD HOOK THREAD
// ----------------------------
DWORD WINAPI HookThread(LPVOID lpParam)
{
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

    if (!keyboardHook)
    {
        SendEvent("HOOK_FAILED");
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // Keep hook alive
    }

    UnhookWindowsHookEx(keyboardHook);
    return 0;
}

// ----------------------------
// FOCUS MONITOR THREAD (WIN-11)
// ----------------------------
DWORD WINAPI FocusMonitorThread(LPVOID lpParam)
{
    focusMonitoring = true;

    while (focusMonitoring)
    {
        HWND foreground = GetForegroundWindow();

        if (appWindow != NULL && foreground != appWindow)
        {
            SendEvent("FOCUS_LOST");
        }

        Sleep(500); // check every 0.5 sec
    }

    return 0;
}

// ----------------------------
// SET APP WINDOW HANDLE
// ----------------------------
Napi::Value SetAppWindow(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsNumber())
    {
        Napi::TypeError::New(env, "HWND expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    appWindow = (HWND)(uintptr_t)info[0].As<Napi::Number>().Int64Value();

    return env.Null();
}

// ----------------------------
// REGISTER JS CALLBACK
// ----------------------------
Napi::Value OnKeyEvent(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (!info[0].IsFunction())
    {
        Napi::TypeError::New(env, "Callback expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    tsfn = Napi::ThreadSafeFunction::New(
        env,
        info[0].As<Napi::Function>(),
        "KeyEventCallback",
        0,
        1);

    return env.Null();
}

// ----------------------------
// START HOOKS
// ----------------------------
Napi::Value StartHook(const Napi::CallbackInfo &info)
{
    CreateThread(NULL, 0, HookThread, NULL, 0, NULL);

    // Start focus monitor
    CreateThread(NULL, 0, FocusMonitorThread, NULL, 0, NULL);

    return info.Env().Null();
}

// ----------------------------
// STOP HOOKS
// ----------------------------
Napi::Value StopHook(const Napi::CallbackInfo &info)
{
    if (keyboardHook)
    {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }

    focusMonitoring = false;

    if (tsfn)
    {
        tsfn.Release();
    }

    return info.Env().Null();
}

// ----------------------------
// MODULE EXPORT
// ----------------------------
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("onKeyEvent", Napi::Function::New(env, OnKeyEvent));
    exports.Set("startHook", Napi::Function::New(env, StartHook));
    exports.Set("stopHook", Napi::Function::New(env, StopHook));
    exports.Set("setAppWindow", Napi::Function::New(env, SetAppWindow));

    return exports;
}

NODE_API_MODULE(addon, Init)