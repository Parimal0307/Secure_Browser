#include <napi.h>
#include <windows.h>
#include <string>

// Global hook + TSFN
HHOOK keyboardHook = NULL;
Napi::ThreadSafeFunction tsfn;

// Forward declaration
void SendEvent(const std::string &msg);

// Keyboard hook procedure
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

        bool isAltPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
        bool isCtrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        bool isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

        // Detect Alt + Tab
        if (p->vkCode == VK_TAB && isAltPressed)
        {
            SendEvent("ALT_TAB_BLOCKED");
            return 1; // BLOCKS the key
        }

        // Alt + Esc
        if (p->vkCode == VK_ESCAPE && isAltPressed)
        {
            SendEvent("ALT_ESC_BLOCKED");
            return 1;
        }

        // Alt + F4 (Close window)
        if (p->vkCode == VK_F4 && isAltPressed)
        {
            SendEvent("ALT_F4_BLOCKED");
            return 1;
        }

        // Detect Windows key
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

        // Ctrl + Esc (Start Menu)
        if (p->vkCode == VK_ESCAPE && isCtrlPressed)
        {
            SendEvent("CTRL_ESC_BLOCKED");
            return 1;
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// Thread function to run hook loop
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
        // Message loop required for hook to stay alive
    }

    UnhookWindowsHookEx(keyboardHook);
    return 0;
}

// Safe bridge: Native thread → JS
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

// JS: register callback
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
        info[0].As<Napi::Function>(), // JS callback
        "KeyEventCallback",
        0, // unlimited queue
        1  // single thread
    );

    return env.Null();
}

// JS: start hook
Napi::Value StartHook(const Napi::CallbackInfo &info)
{
    CreateThread(NULL, 0, HookThread, NULL, 0, NULL);
    return info.Env().Null();
}

// JS: stop hook (cleanup)
Napi::Value StopHook(const Napi::CallbackInfo &info)
{
    if (keyboardHook)
    {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }

    if (tsfn)
    {
        tsfn.Release();
    }

    return info.Env().Null();
}

// Module export
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("onKeyEvent", Napi::Function::New(env, OnKeyEvent));
    exports.Set("startHook", Napi::Function::New(env, StartHook));
    exports.Set("stopHook", Napi::Function::New(env, StopHook));
    return exports;
}

NODE_API_MODULE(addon, Init)