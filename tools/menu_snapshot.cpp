// Render the actual native menu to PNG using an isolated, hidden D3D11 fixture.
// No game is started and no preview symbols are present in the production ASI.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <wincodec.h>
#include <cstdio>
#include <string>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "menu.h"
namespace fgvk {void MenuPreviewConfigure(int,int);void MenuPreviewBounds(float*);}
template<class T> void release(T*& p){if(p){p->Release();p=nullptr;}}
bool save(ID3D11Device* device,ID3D11DeviceContext* context,ID3D11Texture2D* texture,const wchar_t* path){
 D3D11_TEXTURE2D_DESC desc{};texture->GetDesc(&desc);
 float bounds[4]{};fgvk::MenuPreviewBounds(bounds);
 const UINT left=UINT(bounds[0]>4?bounds[0]-4:0),top=UINT(bounds[1]>4?bounds[1]-4:0);
 const UINT right=(std::min)(desc.Width,UINT(bounds[0]+bounds[2]+4)),bottom=(std::min)(desc.Height,UINT(bounds[1]+bounds[3]+4));
 if(right<=left || bottom<=top)return false;
 desc.Width=right-left;desc.Height=bottom-top;
 desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
 ID3D11Texture2D* staging{};if(FAILED(device->CreateTexture2D(&desc,nullptr,&staging)))return false;
 D3D11_BOX box{left,top,0,right,bottom,1};context->CopySubresourceRegion(staging,0,0,0,0,texture,0,&box);D3D11_MAPPED_SUBRESOURCE mapped{};
 if(FAILED(context->Map(staging,0,D3D11_MAP_READ,0,&mapped))){release(staging);return false;}
 IWICImagingFactory* factory{};IWICStream* stream{};IWICBitmapEncoder* encoder{};IWICBitmapFrameEncode* frame{};
 HRESULT result=CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&factory));
 if(SUCCEEDED(result))result=factory->CreateStream(&stream);
 if(SUCCEEDED(result))result=stream->InitializeFromFilename(path,GENERIC_WRITE);
 if(SUCCEEDED(result))result=factory->CreateEncoder(GUID_ContainerFormatPng,nullptr,&encoder);
 if(SUCCEEDED(result))result=encoder->Initialize(stream,WICBitmapEncoderNoCache);
 if(SUCCEEDED(result))result=encoder->CreateNewFrame(&frame,nullptr);
 if(SUCCEEDED(result))result=frame->Initialize(nullptr);
 if(SUCCEEDED(result))result=frame->SetSize(desc.Width,desc.Height);
 WICPixelFormatGUID format=GUID_WICPixelFormat32bppBGRA;
 if(SUCCEEDED(result))result=frame->SetPixelFormat(&format);
 if(SUCCEEDED(result) && !IsEqualGUID(format,GUID_WICPixelFormat32bppBGRA))result=E_FAIL;
 if(SUCCEEDED(result))result=frame->WritePixels(desc.Height,mapped.RowPitch,mapped.RowPitch*desc.Height,(BYTE*)mapped.pData);
 if(SUCCEEDED(result))result=frame->Commit();
 if(SUCCEEDED(result))result=encoder->Commit();
 release(frame);release(encoder);release(stream);release(factory);context->Unmap(staging,0);release(staging);
 return SUCCEEDED(result);
}
int main(int argc,char** argv){
 if(argc!=2)return 1;
 const int length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,argv[1],-1,nullptr,0);
 if(length<=0)return 1;
 std::wstring directory(size_t(length),L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,argv[1],-1,directory.data(),length);directory.pop_back();
 CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
 WNDCLASSW wc{};wc.lpfnWndProc=DefWindowProcW;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"RDR2CustomMenuSnapshot";
 RegisterClassW(&wc);
 HWND window=CreateWindowW(wc.lpszClassName,L"Isolated menu render",WS_OVERLAPPEDWINDOW,0,0,1320,1440,nullptr,nullptr,wc.hInstance,nullptr);
 if(!window)return 2;
 ID3D11Device* device{};ID3D11DeviceContext* context{};
 if(FAILED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)))return 3;
 fgvk::MenuBindWindow(window);if(!fgvk::MenuInitializeUI())return 4;
 if(!ImGui_ImplDX11_Init(device,context))return 5;
 RECT client{};GetClientRect(window,&client);
 D3D11_TEXTURE2D_DESC desc{};desc.Width=client.right;desc.Height=client.bottom;desc.MipLevels=desc.ArraySize=1;
 desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_RENDER_TARGET;
 ID3D11Texture2D* texture{};ID3D11RenderTargetView* target{};
 if(FAILED(device->CreateTexture2D(&desc,nullptr,&texture)) || FAILED(device->CreateRenderTargetView(texture,nullptr,&target)))return 6;
 const wchar_t* names[]={L"generation",L"display",L"diagnostics"};const float background[]={.055f,.06f,.065f,1.f};
 for(int scale=0;scale<=2;scale+=2)for(int page=0;page<3;++page){
  for(int frame=0;frame<4;++frame){
   fgvk::MenuPreviewConfigure(page,scale);ImGui_ImplDX11_NewFrame();fgvk::MenuBuildUI();
   context->OMSetRenderTargets(1,&target,nullptr);context->ClearRenderTargetView(target,background);
   ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }
  std::wstring path=directory;path+=L"/menu-";path+=names[page];if(scale)path+=L"-150";path+=L".png";
  if(!save(device,context,texture,path.c_str()))return 7;
 }
 ImGui_ImplDX11_Shutdown();fgvk::MenuResetUI();release(target);release(texture);release(context);release(device);
 DestroyWindow(window);UnregisterClassW(wc.lpszClassName,wc.hInstance);CoUninitialize();
 puts("Native menu snapshots: Generation, Display, Diagnostics at 100% and 150% rendered successfully.");
}
