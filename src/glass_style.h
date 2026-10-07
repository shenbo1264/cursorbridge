#pragma once
#include <dwmapi.h>
#include <gdiplus.h>
#include <uxtheme.h>
#include <memory>

// System acrylic supplies the blur. This canvas preserves premultiplied alpha
// instead of painting an opaque GDI background over the extended DWM frame.
struct GlassRuntime {
    ULONG_PTR token=0;
    GlassRuntime(){Gdiplus::GdiplusStartupInput input;Gdiplus::GdiplusStartup(&token,&input,NULL);}
    ~GlassRuntime(){if(token)Gdiplus::GdiplusShutdown(token);}
};
class GlassCanvas {
    HDC target,dc;HBITMAP dib;HGDIOBJ previous;void* pixels=NULL;
    int width,height;
public:
    std::unique_ptr<Gdiplus::Bitmap> bitmap;
    std::unique_ptr<Gdiplus::Graphics> graphics;
    GlassCanvas(HDC output,int w,int h):target(output),width(w),height(h){
        BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth=w;info.bmiHeader.biHeight=-h;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
        dc=CreateCompatibleDC(output);dib=CreateDIBSection(output,&info,DIB_RGB_COLORS,&pixels,NULL,0);
        previous=SelectObject(dc,dib);
        bitmap=std::make_unique<Gdiplus::Bitmap>(w,h,w*4,PixelFormat32bppPARGB,(BYTE*)pixels);
        graphics=std::make_unique<Gdiplus::Graphics>(bitmap.get());
        graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
        graphics->Clear(Gdiplus::Color(0,0,0,0));
    }
    ~GlassCanvas(){graphics.reset();bitmap.reset();SelectObject(dc,previous);DeleteObject(dib);DeleteDC(dc);}
    HDC NativeDC(){graphics->Flush(Gdiplus::FlushIntentionSync);return dc;}
    void MakeOpaque(){for(int i=0;i<width*height;i++)((DWORD*)pixels)[i]|=0xff000000;}
    void Present(){graphics->Flush(Gdiplus::FlushIntentionSync);BitBlt(target,0,0,width,height,dc,0,0,SRCCOPY);}
    void Icon(HCURSOR icon,int x,int y,int size,UINT frame){
        // Cursor previews sit on an opaque well: GDI cursor drawing can clear
        // alpha, so repair only that well, never the translucent window surface.
        graphics->Flush(Gdiplus::FlushIntentionSync);
        DrawIconEx(dc,x,y,icon,size,size,frame,NULL,DI_NORMAL);
        for(int row=(std::max)(0,y);row<(std::min)(height,y+size);row++)
            for(int col=(std::max)(0,x);col<(std::min)(width,x+size);col++)((DWORD*)pixels)[row*width+col]|=0xff000000;
    }
};
inline Gdiplus::Color GlassColor(COLORREF color,BYTE alpha=255){return Gdiplus::Color(alpha,GetRValue(color),GetGValue(color),GetBValue(color));}
inline void GlassRound(Gdiplus::Graphics& g,RECT rect,float radius,Gdiplus::Color fill,Gdiplus::Color stroke=Gdiplus::Color(0,0,0,0),float weight=1){
    using namespace Gdiplus;
    float x=(float)rect.left,y=(float)rect.top,w=(float)(rect.right-rect.left),h=(float)(rect.bottom-rect.top);
    float d=(std::min)(radius*2,(std::min)(w,h));GraphicsPath path;
    path.AddArc(x,y,d,d,180,90);path.AddArc(x+w-d,y,d,d,270,90);
    path.AddArc(x+w-d,y+h-d,d,d,0,90);path.AddArc(x,y+h-d,d,d,90,90);path.CloseFigure();
    SolidBrush brush(fill);g.FillPath(&brush,&path);if(stroke.GetA()){Pen pen(stroke,weight);g.DrawPath(&pen,&path);}
}
inline bool GlassHighContrast(){HIGHCONTRASTW value={};value.cbSize=sizeof(value);return SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(value),&value,0)&&(value.dwFlags&HCF_HIGHCONTRASTON);}
inline bool GlassTransparencyAllowed(){
    DWORD value=1,size=sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",L"EnableTransparency",RRF_RT_REG_DWORD,NULL,&value,&size);
    BOOL composition=FALSE;return value!=0&&!GlassHighContrast()&&SUCCEEDED(DwmIsCompositionEnabled(&composition))&&composition;
}
inline bool GlassApplyBackdrop(HWND window){
    // Attribute numbers keep the executable loadable with older Windows SDKs
    // and on Windows 10. Unsupported documented attributes return E_INVALIDARG.
    BOOL dark=FALSE;DwmSetWindowAttribute(window,20,&dark,sizeof(dark));
    int corner=2;DwmSetWindowAttribute(window,33,&corner,sizeof(corner));
    int backdrop=GlassTransparencyAllowed()?3:1;
    HRESULT result=DwmSetWindowAttribute(window,38,&backdrop,sizeof(backdrop));
    bool active=backdrop==3&&SUCCEEDED(result);MARGINS margins=active?MARGINS{-1,-1,-1,-1}:MARGINS{0,0,0,0};
    if(FAILED(DwmExtendFrameIntoClientArea(window,&margins)))active=false;
    return active;
}
