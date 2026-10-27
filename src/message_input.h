#pragma once
#include <windows.h>
#include <atomic>
#include <cstdint>
namespace fgvk::menu_input {
using Handler=LRESULT(*)(HWND,UINT,WPARAM,LPARAM);
inline HHOOK hook{};inline HWND target{};inline Handler handler{};
inline HHOOK focusHook{};
inline std::atomic<uint64_t> focusLosses{0};
inline LRESULT CALLBACK FocusProcedure(int code,WPARAM w,LPARAM data){
  if(code>=0 && data){
    const auto* message=reinterpret_cast<const CWPSTRUCT*>(data);
    if(message->hwnd==target &&
       ((message->message==WM_ACTIVATEAPP && !message->wParam) ||
        (message->message==WM_ACTIVATE && LOWORD(message->wParam)==WA_INACTIVE) ||
        message->message==WM_KILLFOCUS ||
        (message->message==WM_SIZE && message->wParam==SIZE_MINIMIZED)))
      focusLosses.fetch_add(1,std::memory_order_release);
  }
  // Observation only: activation, keyboard and raw input still reach RDR2.
  return CallNextHookEx(focusHook,code,w,data);
}
inline void Uninstall(){
  if(focusHook)UnhookWindowsHookEx(focusHook);
  if(hook)UnhookWindowsHookEx(hook);
  focusHook=hook=nullptr;target=nullptr;handler=nullptr;
}
inline std::atomic<unsigned> nestedCallbacks{0};
inline thread_local bool inCallback=false;
inline bool CanConsume(UINT message){
  // The overlay borrows the host HWND. Raw device input, releases, activation
  // and lifecycle messages must still reach the game's own input owner.
  if(message==WM_KEYUP || message==WM_SYSKEYUP || message==WM_SYSKEYDOWN ||
     message==WM_LBUTTONUP || message==WM_RBUTTONUP || message==WM_MBUTTONUP || message==WM_XBUTTONUP)return false;
  return message==WM_KEYDOWN || message==WM_CHAR ||
    message==WM_MOUSEMOVE || (message>=WM_LBUTTONDOWN && message<=WM_MOUSEHWHEEL);
}
inline LRESULT CALLBACK QueueProcedure(int code,WPARAM remove,LPARAM data){
  if(code<0)return CallNextHookEx(hook,code,remove,data);
  // WH_GETMESSAGE does not use the callback's return value to dispatch MSG.
  // Leave a nested message intact, without re-entering the same hook chain.
  if(inCallback){nestedCallbacks.fetch_add(1,std::memory_order_relaxed);return 0;}
  struct Scope {Scope(){inCallback=true;}~Scope(){inCallback=false;}} scope;
  const LRESULT next=CallNextHookEx(hook,code,remove,data);
  if(code>=0 && remove==PM_REMOVE && data){
    auto* msg=reinterpret_cast<MSG*>(data);
    if(msg->hwnd==target && handler){
      const bool consumed=handler(msg->hwnd,msg->message,msg->wParam,msg->lParam)==0;
      if(consumed && CanConsume(msg->message)){msg->message=WM_NULL;msg->wParam=0;msg->lParam=0;}
    }
  }
  return next;
}
inline bool Install(HWND window,HMODULE module,Handler callback){
  if(hook){const bool same=target==window && handler==callback;if(!same)SetLastError(ERROR_ALREADY_EXISTS);return same;}
  DWORD process{};const DWORD thread=GetWindowThreadProcessId(window,&process);
  if(!window || !module || !callback || !thread || process!=GetCurrentProcessId()){SetLastError(ERROR_INVALID_PARAMETER);return false;}
  target=window;handler=callback;
  hook=SetWindowsHookExW(WH_GETMESSAGE,QueueProcedure,module,thread);
  if(hook)focusHook=SetWindowsHookExW(WH_CALLWNDPROC,FocusProcedure,module,thread);
  return hook!=nullptr;
}
}
