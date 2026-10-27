#include "message_input.h"
#include <cassert>
#include <cstdio>
#include <initializer_list>
static int handled=0;
static int focusDelivered=0;
static bool captureEverything=false;
static LRESULT Input(HWND,UINT msg,WPARAM,LPARAM){if(msg==WM_KEYDOWN){++handled;return 0;}return captureEverything?0:1;}
static LRESULT CALLBACK Owner(HWND w,UINT m,WPARAM a,LPARAM b){if(m==WM_KILLFOCUS)++focusDelivered;return DefWindowProcW(w,m,a,b);}
int main(){
 WNDCLASSW wc{};wc.hInstance=GetModuleHandleW(nullptr);wc.lpfnWndProc=Owner;wc.lpszClassName=L"FGInputFixture";
 assert(RegisterClassW(&wc));HWND w=CreateWindowW(wc.lpszClassName,L"Fixture",0,0,0,20,20,nullptr,nullptr,wc.hInstance,nullptr);assert(w);
 const auto before=GetWindowLongPtrW(w,GWLP_WNDPROC);
 assert(!fgvk::menu_input::Install(nullptr,wc.hInstance,Input));assert(GetLastError()==ERROR_INVALID_PARAMETER);
 assert(fgvk::menu_input::Install(w,wc.hInstance,Input));assert(GetWindowLongPtrW(w,GWLP_WNDPROC)==before);
 const auto hook=fgvk::menu_input::hook;assert(fgvk::menu_input::Install(w,wc.hInstance,Input));assert(fgvk::menu_input::hook==hook);
 assert(fgvk::menu_input::focusHook);
 auto losses=fgvk::menu_input::focusLosses.load();
 SendMessageW(w,WM_KILLFOCUS,0,0);
 assert(fgvk::menu_input::focusLosses.load()==losses+1 && focusDelivered==1);
 MSG msg{};while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){}
 MSG nested{};nested.hwnd=w;nested.message=WM_KEYDOWN;nested.wParam=VK_INSERT;
 fgvk::menu_input::inCallback=true;
 fgvk::menu_input::QueueProcedure(0,PM_REMOVE,reinterpret_cast<LPARAM>(&nested));
 fgvk::menu_input::inCallback=false;
 assert(nested.message==WM_KEYDOWN && handled==0 && fgvk::menu_input::nestedCallbacks==1);
 assert(PostMessageW(w,WM_KEYDOWN,VK_INSERT,0));assert(PeekMessageW(&msg,w,0,0,PM_NOREMOVE));assert(handled==0);
 assert(PeekMessageW(&msg,w,0,0,PM_REMOVE));assert(handled==1);assert(msg.message==WM_NULL);
 assert(PostMessageW(w,WM_APP,7,9));assert(PeekMessageW(&msg,w,0,0,PM_REMOVE));assert(msg.message==WM_APP && msg.wParam==7 && msg.lParam==9);
 // Even an over-eager overlay handler cannot discard the game's raw-input
 // packets, releases, focus, capture or lifecycle messages. Preserve payloads.
 captureEverything=true;
 for(UINT message:{WM_INPUT,WM_KEYUP,WM_SYSKEYUP,WM_SYSKEYDOWN,WM_LBUTTONUP,WM_RBUTTONUP,WM_MBUTTONUP,WM_XBUTTONUP,WM_ACTIVATE,WM_ACTIVATEAPP,WM_KILLFOCUS,WM_SETFOCUS,WM_CAPTURECHANGED,WM_SIZE,WM_SYSCOMMAND,WM_APP}){
   MSG protectedMessage{};protectedMessage.hwnd=w;protectedMessage.message=message;protectedMessage.wParam=7;protectedMessage.lParam=9;
   fgvk::menu_input::QueueProcedure(0,PM_REMOVE,reinterpret_cast<LPARAM>(&protectedMessage));
   if(protectedMessage.message!=message || protectedMessage.wParam!=7 || protectedMessage.lParam!=9){
     puts("FAIL: overlay discarded a host input/lifecycle message");
     fgvk::menu_input::Uninstall();DestroyWindow(w);return 1;
   }
 }
 captureEverything=false;
 assert(GetWindowLongPtrW(w,GWLP_WNDPROC)==before);
 fgvk::menu_input::Uninstall();assert(!fgvk::menu_input::hook && !fgvk::menu_input::focusHook);
 SendMessageW(w,WM_KILLFOCUS,0,0);assert(fgvk::menu_input::focusLosses.load()==losses+1 && focusDelivered==2);
 DestroyWindow(w);puts("Menu input: raw input, releases, focus/capture/lifecycle preserved; sent focus observer, hook cleanup and host WndProc unchanged: PASS");
}
