#include <windows.h>
#include <math.h>
#include <iostream>
#include <thread>

double intensity = 0.0;
bool state = false;

double fade(double maxIntensity) {
    if (state == false) {
        intensity += 1.0;
        if (intensity >= maxIntensity) {
            state = true;
        }
    }
    else {
        intensity -= 1.0;
        if (intensity <= 0) {
            state = false;
        }
    }
    return intensity;
}

DWORD WINAPI rainbowShader3(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    //float f = 0.0;

    HDC hdc = GetDC(0);

    while (true) {
        for (int i = 0; i < y; i++) {
            //f = fade(70);
            SelectObject(hdc, CreateSolidBrush(RGB(fade(100), fade(100), fade(100))));
            //BitBlt(hdc, f, i, x, 1, hdc, 0, i, SRCCOPY);
            BitBlt(hdc, 0, 0, x, y, hdc, 0, 0, PATINVERT);
            Sleep(100);
        }
    }
}

void RGBtoHSV(COLORREF rgb, double& h, double& s, double& v) {
    double r = GetRValue(rgb) / 255.0;
    double g = GetGValue(rgb) / 255.0;
    double b = GetBValue(rgb) / 255.0;

    double cmax = max(r, max(g, b));
    double cmin = min(r, min(g, b));
    double delta = cmax - cmin;

    if (delta == 0) {
        h = 0;
    }
    else if (cmax == r) {
        h = 60.0 * fmod(((g - b) / delta), 6.0);
    }
    else if (cmax == g) {
        h = 60.0 * (((b - r) / delta) + 2.0);
    }
    else {
        h = 60.0 * (((r - g) / delta) + 4.0);
    }

    if (h < 0) {
        h += 360.0;
    }

    s = (cmax == 0) ? 0 : delta / cmax;
    v = cmax;
}

COLORREF HSVtoRGB(double h, double s, double v) {
    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - c;

    double r, g, b;
    if (h < 60) {
        r = c;
        g = x;
        b = 0;
    }
    else if (h < 120) {
        r = x;
        g = c;
        b = 0;
    }
    else if (h < 180) {
        r = 0;
        g = c;
        b = x;
    }
    else if (h < 240) {
        r = 0;
        g = x;
        b = c;
    }
    else if (h < 300) {
        r = x;
        g = 0;
        b = c;
    }
    else {
        r = c;
        g = 0;
        b = x;
    }

    int red = static_cast<int>((r + m) * 255);
    int green = static_cast<int>((g + m) * 255);
    int blue = static_cast<int>((b + m) * 255);

    return RGB(red, green, blue);
}

DWORD WINAPI hsvlines(LPVOID lpParam) {
    Sleep(2000);
    int v = 0;
    while (true) {
        int x = GetSystemMetrics(SM_CXSCREEN);
        int y = GetSystemMetrics(SM_CYSCREEN);

        COLORREF* pixels = new COLORREF[x * y];

        HDC hdc = GetDC(0);
        HDC mdc = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
        SelectObject(mdc, bmp);
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);

        BITMAPINFOHEADER bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = x;
        bi.biHeight = y;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        GetDIBits(mdc, bmp, 0, y, pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        double hue = 0, saturation = 0, lightness = 0;
        for (int i = 0; i < x * y; i++) {
            int x2 = i % x, y2 = i / y;
            RGBtoHSV(pixels[i], hue, saturation, lightness);

            if (!(i % y) && !(rand() % 200))
                v = rand() % 12;
            hue = fmod(hue + (v % 360), 360);

            pixels[i] = HSVtoRGB(hue, saturation, lightness);
        }

        SetDIBits(mdc, bmp, 0, y, pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        DeleteObject(bmp);
        DeleteDC(mdc);
        ReleaseDC(0, hdc);

        delete[] pixels;
    }
}

typedef union COLOR {
    COLORREF rgb;
    COLORREF hsv;
};

int red, green, blue;

COLORREF COLORRGB(int length, int speed) {
    if (red < length) {
        red += speed;

        return RGB(red, 0, length);
    }
    else if (green < length) {
        green += speed;

        return RGB(length, green, 0);
    }
    else if (blue < length) {
        blue += speed;

        return RGB(0, length, blue);
    }
    else {
        red = 0; green = 0; blue = 0;

        return RGB(0, length, length);
    }
}

COLORREF COLORHSL(int length) {
    double h = fmod(length, 360.0);
    double s = 1.0;
    double l = 0.5;

    double c = (1.0 - fabs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c / 2.0;

    double r1, g1, b1;
    if (h < 60) {
        r1 = c;
        g1 = x;
        b1 = 0;
    }
    else if (h < 120) {
        r1 = x;
        g1 = c;
        b1 = 0;
    }
    else if (h < 180) {
        r1 = 0;
        g1 = c;
        b1 = x;
    }
    else if (h < 240) {
        r1 = 0;
        g1 = x;
        b1 = c;
    }
    else if (h < 300) {
        r1 = x;
        g1 = 0;
        b1 = c;
    }
    else {
        r1 = c;
        g1 = 0;
        b1 = x;
    }

    int red = static_cast<int>((r1 + m) * 255);
    int green = static_cast<int>((g1 + m) * 255);
    int blue = static_cast<int>((b1 + m) * 255);

    return RGB(red, green, blue);
}

VOID WINAPI KillPayload(HANDLE h) {
    TerminateThread(h, 0);
    CloseHandle(h);
    //HeapDestroy(hHeap);
}

VOID WINAPI RedrawScreen(HWND hwnd) {
    RedrawWindow(hwnd, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

DWORD WINAPI firstPayload(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int pixelSize = 5;
    int j = 0;

    RGBQUAD* data = (RGBQUAD*)VirtualAlloc(0, (x * y + x) * sizeof(RGBQUAD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 50;

    while (true) {
        hdc = GetDC(0);

        //StretchBlt(mdc, 0, 0, x / 3, y / 3, hdc, 0, 0, x, y, SRCCOPY);
        StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, SRCPAINT);
        GetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);

        /*
        for (int i = 0; i < x * y; ++i) {
            int x2 = i % x;
            int y2 = i / y;

            BYTE gray = (GetRValue(data[i].rgb) + GetGValue(data[i].rgb) + GetBValue(data[i].rgb)) / 3;

            double hue, saturation, lightness;
            RGBtoHSV(data[i].hsv, hue, saturation, lightness);

            hue = fmod(hue + (x2 & y2) % 360, 360);

            data[i].hsv = HSVtoRGB(hue, saturation, lightness);
        }
        */

        for (int yBlock = 0; yBlock < y; yBlock += pixelSize) {
            for (int xBlock = 0; xBlock < x; xBlock += pixelSize) {
                int red = 0, green = 0, blue = 0, count = 0;
                for (int py = 0; py < pixelSize && (yBlock + py) < y; ++py) {
                    for (int px = 0; px < pixelSize && (xBlock + px) < x; ++px) {
                        int index = ((yBlock + py) * x + (xBlock + px));
                        blue += data[index].rgbBlue;
                        green += data[index].rgbGreen;
                        red += data[index].rgbRed;
                        ++count;
                    }
                }
                if (count > 0) {
                    blue /= count;
                    green /= count;
                    red /= count;
                }
                for (int py = 0; py < pixelSize && (yBlock + py) < y; ++py) {
                    for (int px = 0; px < pixelSize && (xBlock + px) < x; ++px) {
                        int index = ((yBlock + py) * x + (xBlock + px));
                        data[index].rgbBlue = blue;
                        data[index].rgbGreen = green;
                        data[index].rgbRed = red;
                    }
                }
            }
        }

        for (int i = 0; i < x * y; i++) {
            data[i].rgbBlue += j;
            data[i].rgbGreen += j;
            data[i].rgbRed += j;
        }

        SetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);
        //GdiAlphaBlend(hdc, 0, 0, x, y, mdc, 0, 0, x / 3, y / 3, blur);
        GdiAlphaBlend(hdc, 0, 0, x, y, mdc, 0, 0, x, y, blur);
        Sleep(500);

        if (rand() % 3 == 0) {
            RedrawScreen(0);
            Sleep(20);
        }

        j++;
    }
    return 0;
}

void InvertColor(DWORD& color) {
    BYTE* bytes = (BYTE*)&color;
    bytes[0] = ~bytes[0];
    bytes[1] = ~bytes[1];
    bytes[2] = ~bytes[2];
}

DWORD WINAPI stretch(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int rx = 0;
    int ry = 0;

    HDC hdc = GetDC(0);

    while (true) {
        hdc = GetDC(0);

        BitBlt(hdc, -5, -5, x / 2, y / 2, hdc, 0, 0, SRCAND); //0x92b246a8clu

        BitBlt(hdc, x / 2 + 5, -5, x / 2, y / 2, hdc, x / 2, 0, SRCAND);

        BitBlt(hdc, -5, y / 2 + 5, x / 2, y / 2, hdc, 0, y / 2, SRCAND);

        BitBlt(hdc, x / 2 + 5, y / 2 + 5, x / 2, y / 2, hdc, x / 2, y / 2, SRCAND);
        
        Sleep(20);

        if (rand() % 5 == 0) {
            RedrawScreen(0);
        }
    }
    return 0;
}

DWORD WINAPI secondPayload(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = AC_SRC_ALPHA;
    blur.SourceConstantAlpha = 127;

    for (int i = 0; i < 50; i++) {
        hdc = GetDC(GetForegroundWindow());

        StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, 0xCE646Cu);

        GdiAlphaBlend(hdc, rand() % 30 - (10), rand() % 30 - (10), x, y, mdc, 0, 0, x, y, blur);
        Sleep(50);

        if (rand() % 5 == 0) {
            RedrawScreen(GetForegroundWindow());
            Sleep(20);
        }
    }
    while (true) {
        for (int i = 0; i < 10; i++) {
            hdc = GetDC(0);

            StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, SRCCOPY);

            GdiAlphaBlend(hdc, rand() % 30 - (10), rand() % 30 - (10), x, y, mdc, 0, 0, x, y, blur);
            Sleep(50);

            if (rand() % 5 == 0) {
                RedrawScreen(0);
                Sleep(20);
            }
        }
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < x + y; j++) {
                float angle = (sin((i + j) * (3.14159 / 100)) * 50) + fade(50) + random() % 30;

                BitBlt(hdc, angle, j, x, 1, mdc, 0, j, SRCCOPY);
            }
            Sleep(50);
        }
        for (int i = 0; i < 3; i++) {
            hbrush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
            
            SelectObject(hdc, hbrush);
            BitBlt(hdc, rand() % 30 - (10), rand() % 30 - (10), x, y, hdc, 0, 0, 0xCE646Cu);
            Sleep(50);
        }
        for (int i = 0; i < x; i++) {
            //StretchBlt(hdc, -10 - j, 0, x + 20, y, hdc, 0, 0, x, y, SRCCOPY);
            StretchBlt(hdc, -10, 0, i + 20, y, mdc, 0, 0, x, y, SRCCOPY);
        }
        for (int i = 0; i < 10; i++) {
            hdc = GetDC(GetForegroundWindow());

            StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, 0xCE646Cu);

            GdiAlphaBlend(hdc, rand() % 30 - (10), rand() % 30 - (10), x, y, mdc, 0, 0, x, y, blur);
            Sleep(50);

            if (rand() % 5 == 0) {
                RedrawScreen(GetForegroundWindow());
                Sleep(20);
            }
        }
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
    }
    return 0;
}

void delay(unsigned int milliseconds) {
    auto start = std::chrono::high_resolution_clock::now();

    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if (elapsed >= milliseconds) {
            break;
        }

        std::this_thread::yield();
    }
}

DWORD WINAPI xWaves(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int size = 0;

    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    while (true) {
        hdc = GetDC(0);

        for (int i = 0; i < 10; i++) {
            StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, SRCCOPY);

            size = rand() % 360;

            for (int j = 0; j < x + y; j++) {
                float angle = (sin((i + j) * (3.14159 / (size * 2))) * size) + fade(size);

                BitBlt(hdc, j, angle, 1, y, mdc, j, 0, SRCCOPY);
                delay(1);
            }
            Sleep(50);
        }
    }
    return 0;
}

DWORD WINAPI alphaBlur(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    //int rx = 0;
    //int ry = 0;

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 127;

    while (true) {
        hdc = GetDC(0);

        StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, SRCCOPY);

        //rx = rand() % (x - 200);
        //ry = rand() % (y - 200);

        GdiAlphaBlend(hdc, rand() % 30 - (10), rand() % 30 - (10), x, y, mdc, 0, 0, x, y, blur);
        Sleep(30);

        if (rand() % 9 == 0) {
            RedrawScreen(0);
            Sleep(20);
        }
    }
}

DWORD WINAPI hueNoShift(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int color = 0;

    COLOR* data = (COLOR*)VirtualAlloc(0, x * y * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    while (true) {
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

        color = rand() % 360;

        for (int i = 0; i < x * y; i++) {
            //BitBlt(hdc, rand() % 30 - (10), rand() % 30 - (10), x, y, hdc, 0, 0, (0xCE646C + MERGECOPY) & SRCPAINT);

            double hue, saturation, lightness;
            RGBtoHSV(data[i].hsv, hue, saturation, lightness);

            hue = fmod(color, 360);

            data[i].hsv = HSVtoRGB(hue, saturation, lightness);
        }

        SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        Sleep(rand() % 500);
    }
}

DWORD WINAPI SquaresRGB(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int rx = 0;
    int ry = 0;

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 64;

    while (true) {
        for (int i = 0; i < 4; i++) {
            rx = rand() % (x - 200);
            ry = rand() % (y - 200);

            BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);

            hbrush = CreateSolidBrush(COLORHSL(fade(360) * 3));

            SelectObject(mdc, hbrush);
            PatBlt(mdc, rx, ry, 200, 200, PATCOPY);

            GdiAlphaBlend(hdc, 0, 0, x, y, mdc, 0, 0, x, y, blur);

            Sleep(100);
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

DWORD WINAPI thirdPayload(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int v = 0;

    LPVOID* data = new LPVOID[x * y];
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    while (true) {
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, 4 * y * x, data);

        for (int i = 0; i < x * y; ++i) {
            if (!(sin(100 * fade(v))))
                v = fade(24);
            ((BYTE*)(data + i))[v % 24] += (((BYTE*)(data + i + v))[v] + *((BYTE*)data + 4 * i + v)) * 3;
        }

        SetBitmapBits(bmp, x * y * 4, data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);
        Sleep(30);

        if (rand() % 5 == 0) {
            RedrawScreen(0);
            Sleep(20);
        }
    }
    return 0;
}

DWORD WINAPI touhouPayload(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int j = 0;

    COLOR* data = (COLOR*)VirtualAlloc(0, (x * y + x) * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 50;

    while (true) {
        hdc = GetDC(0);

        StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, SRCCOPY);
        GetBitmapBits(bmp, 4 * y * x, data);

        for (int i = 0; i < x * y; ++i) {
            int x2 = i % x;
            int y2 = i / y;

            //BYTE gray = (GetRValue(data[i].rgb) + GetGValue(data[i].rgb) + GetBValue(data[i].rgb)) / 3;
            //data[i].rgb += i + (i % x) ^ 2 + (((BYTE*)(data + i + rand() % 50))[rand() % 50] + *((BYTE*)data + 4 * i + (int)fade(100))) * ((i % x) ^ (i / y));
            
            double hue, saturation, lightness;
            RGBtoHSV(data[i].hsv, hue, saturation, lightness);

            //hue = fmod(hue + (x2 & y2) + j + ((BYTE*)(data + i + 24))[24] % 360, 360);
            hue = fmod(hue + (((x2 + j) & (y2 + j)) / 5) % 360, 360);

            data[i].hsv = HSVtoRGB(hue, saturation, lightness);
            //data[i].rgb += (x2 + y2) ^ (x2 & y2) + (x2 - y2) & (x2 ^ y2) | y2;
        }

        SetBitmapBits(bmp, 4 * y * x, data);
        StretchBlt(hdc, 0, 0, x, y, mdc, 0, 0, x, y, SRCCOPY);
        Sleep(10);

        j++;
    }
    return 0;
}

DWORD WINAPI shapesPayload(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);
    HRGN hrgn;
    int speed = 50;

    while (true) {
        int size = 500;
        int x2 = rand() % (x + size) - size / 2;
        int y2 = rand() % (y + size) - size / 2;


        for (int i = 0; i < 4; i++) {
            x2 = rand() % (x + size) - size / 2;
            y2 = rand() % (y + size) - size / 2;

            for (int j = 100; j <= size; j += 100) {
                hrgn = CreateEllipticRgn(x2 - j / 2, y2 - j / 2, x2 + j / 2, y2 + j / 2);

                SelectClipRgn(hdc, hrgn);
                BitBlt(hdc, x2 - j / 2, y2 - j / 2, j, j, hdc, x2 - j / 2, y2 - j / 2, NOTSRCCOPY);
                DeleteObject(hrgn);

                delay(speed);
            }
        }

        for (int i = 0; i < 4; i++) {
            x2 = rand() % (x + size) - size / 2;
            y2 = rand() % (y + size) - size / 2;

            for (int j = 100; j <= size; j += 100) {
                hrgn = CreateRectRgn(x2 - j / 2, y2 - j / 2, x2 + j / 2, y2 + j / 2);

                SelectClipRgn(hdc, hrgn);
                BitBlt(hdc, x2 - j / 2, y2 - j / 2, j, j, hdc, x2 - j / 2, y2 - j / 2, NOTSRCCOPY);
                DeleteObject(hrgn);

                delay(speed);
            }
        }

        for (int i = 0; i < 4; i++) {
            x2 = rand() % (x + size) - size / 2;
            y2 = rand() % (y + size) - size / 2;

            for (int j = 100; j <= size; j += 100) {
                POINT vertices[6];
                vertices[0].x = x2;
                vertices[0].y = y2 - j / 2;
                vertices[1].x = x2 + j / 2;
                vertices[1].y = y2 - j / 4;
                vertices[2].x = x2 + j / 2;
                vertices[2].y = y2 + j / 4;
                vertices[3].x = x2;
                vertices[3].y = y2 + j / 2;
                vertices[4].x = x2 - j / 2;
                vertices[4].y = y2 + j / 4;
                vertices[5].x = x2 - j / 2;
                vertices[5].y = y2 - j / 4;

                hrgn = CreatePolygonRgn(vertices, 6, WINDING);

                SelectClipRgn(hdc, hrgn);
                BitBlt(hdc, x2 - j / 2, y2 - j / 2, j, j, hdc, x2 - j / 2, y2 - j / 2, NOTSRCCOPY);
                DeleteObject(hrgn);

                delay(speed);
            }
        }

        if (rand() % 5 == 0) {
            RedrawScreen(0);
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

DWORD WINAPI circlePayload(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int maxSize = 0;
    int minSize = 0;

    HDC hdc = GetDC(0);
    HRGN hrgn;
    int speed = 50;

    while (true) {
        maxSize = rand() % 700;
        minSize = rand() % 100;

        int initialSize = rand() % (maxSize - minSize + 1) + minSize;
        int currentSize = initialSize;

        int xPos = rand() % (x + currentSize) - currentSize / 2;
        int yPos = rand() % (y + currentSize) - currentSize / 2;

        for (int size = currentSize; size >= minSize; size -= 50) {
            hrgn = CreateEllipticRgn(xPos - size / 2, yPos - size / 2, xPos + size / 2, yPos + size / 2);

            SelectClipRgn(hdc, hrgn);
            BitBlt(hdc, xPos - size / 2, yPos - size / 2, size, size, hdc, xPos - size / 2, yPos - size / 2, NOTSRCCOPY);

            delay(speed);
        }

        for (int size = minSize; size <= currentSize; size += 50) {
            hrgn = CreateEllipticRgn(xPos - size / 2, yPos - size / 2, xPos + size / 2, yPos + size / 2);

            SelectClipRgn(hdc, hrgn);
            BitBlt(hdc, xPos - size / 2, yPos - size / 2, size, size, hdc, xPos - size / 2, yPos - size / 2, NOTSRCCOPY);

            delay(speed);
        }
    }

    return 0;
}

DWORD WINAPI invert(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    while (true) {
        BitBlt(hdc, 0, 0, x, y, hdc, 0, 0, NOTSRCCOPY);

        Sleep(1000);
    }
    return 0;
}

DWORD WINAPI waves(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int ry = 0;

    HDC hdc = GetDC(0);

    while (true) {
        ry = rand() % y;

        if (rand() % 2 == 1) {
            BitBlt(hdc, 0, ry, x, 15, hdc, 10, ry, SRCCOPY);
        }
        else {
            BitBlt(hdc, 10, ry, x, 15, hdc, 0, ry, SRCCOPY);
        }

        Sleep(2);
    }
    return 0;
}

DWORD WINAPI highWaves(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int r = 0;

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    while (true) {
        hdc = GetDC(0);
        r = rand() % 50;

        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);

        for (int i = 0; i < x + y; i += 10) {
            int wave = sin(i / ((float)x / r) * 3.14159) * (r * 2);

            BitBlt(hdc, 0, i, x, 10, mdc, wave, i, SRCCOPY);
        }

        Sleep(50);
    }
}

struct Point3D {
    float x, y, z;
};

void DrawEllipseAt(HDC hdc, int x, int y, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    SelectObject(hdc, brush);
    Ellipse(hdc, x - 30, y - 30, x + 30, y + 30);  // Draw an ellipse with 50x50 size
    DeleteObject(brush);
}

Point3D RotatePoint(Point3D point, float angleX, float angleY, float angleZ) {
    float cosX = cos(angleX), sinX = sin(angleX);
    float cosY = cos(angleY), sinY = sin(angleY);
    float cosZ = cos(angleZ), sinZ = sin(angleZ);

    float y = point.y * cosX - point.z * sinX;
    float z = point.y * sinX + point.z * cosX;
    point.y = y;
    point.z = z;

    float x = point.x * cosY + point.z * sinY;
    z = -point.x * sinY + point.z * cosY;
    point.x = x;
    point.z = z;

    x = point.x * cosZ - point.y * sinZ;
    y = point.x * sinZ + point.y * cosZ;
    point.x = x;
    point.y = y;

    return point;
}

void Draw3DCube(HDC hdc, Point3D center, float size, float angleX, float angleY, float angleZ, float colorA) {
    Point3D vertices[8] = {
        {-size, -size, -size},
        {size, -size, -size},
        {size, size, -size},
        {-size, size, -size},
        {-size, -size, size},
        {size, -size, size},
        {size, size, size},
        {-size, size, size},
    };

    POINT screenPoints[8];

    for (int i = 0; i < 8; ++i) {
        Point3D rotated = RotatePoint(vertices[i], angleX, angleY, angleZ);
        COLORREF color = COLORHSL(colorA);

        int screenX = static_cast<int>(center.x + rotated.x);
        int screenY = static_cast<int>(center.y + rotated.y);

        screenPoints[i].x = screenX;
        screenPoints[i].y = screenY;

        DrawEllipseAt(hdc, screenX, screenY, color);
    }

    POINT polyline1[5] = { screenPoints[0], screenPoints[1], screenPoints[2], screenPoints[3], screenPoints[0] };
    Polyline(hdc, polyline1, 5);

    POINT polyline2[5] = { screenPoints[4], screenPoints[5], screenPoints[6], screenPoints[7], screenPoints[4] };
    Polyline(hdc, polyline2, 5);

    POINT connectingLines[8] = {
        screenPoints[0], screenPoints[4],
        screenPoints[1], screenPoints[5],
        screenPoints[2], screenPoints[6],
        screenPoints[3], screenPoints[7]
    };
    Polyline(hdc, &connectingLines[0], 2);
    Polyline(hdc, &connectingLines[2], 2);
    Polyline(hdc, &connectingLines[4], 2);
    Polyline(hdc, &connectingLines[6], 2);
}

DWORD WINAPI cube(LPVOID lpParam) {
    int signX = 1;
    int signY = 1;
    int incrementor = 5;
    float x2 = 100.0f;
    float y2 = 100.0f;
    float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
    float angleIncrement = 0.05f;
    float colorA = 0;
    float size = 0.0f;

    while (true) {
        HDC hdc = GetDC(0);
        int x = GetSystemMetrics(SM_CXSCREEN);
        int y = GetSystemMetrics(SM_CYSCREEN);

        x2 += incrementor * signX;
        y2 += incrementor * signY;

        if (x2 + 75 >= x) {
            signX = -1;
            x2 = x - 76;
        }
        else if (x2 <= 75) {
            signX = 1;
            x2 = 76;
        }

        if (y2 + 75 >= y) {
            signY = -1;
            y2 = y - 76;
        }
        else if (y2 <= 75) {
            signY = 1;
            y2 = 76;
        }

        Point3D center = { x2, y2, 0.0f };
        Draw3DCube(hdc, center, size, angleX, angleY, angleZ, colorA);

        angleX += angleIncrement;
        angleY += angleIncrement;
        angleZ += angleIncrement;

        delay(50);

        ReleaseDC(0, hdc);
        colorA += 1;

        if (size >= 0 && size <= 100) {
            size += 0.5;
        }
    }

    return 0;
}

DWORD WINAPI hueCircle(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    COLOR* data = (COLOR*)VirtualAlloc(0, (x * y + x) * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    int centerX = x / 2;
    int centerY = y / 2;
    double maxDistance = sqrt(centerX * centerX + centerY * centerY);

    while (true) {
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

        for (int i = 0; i < x * y; i++) {
            int x2 = i % x;
            int y2 = i / x;

            double distance = sqrt((x2 - centerX) * (x2 - centerX) + (y2 - centerY) * (y2 - centerY));
            double factor = pow(distance / maxDistance, 2);

            double hue, saturation, lightness;
            RGBtoHSV(data[i].rgb, hue, saturation, lightness);

            hue = fmod(hue + (factor * 360), 360);

            data[i].rgb = HSVtoRGB(hue, saturation, lightness);
        }

        SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        Sleep(10);
    }
    return 0;
}

DWORD WINAPI cubes(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int v = 0;
    int r = 0;

    COLOR* data = (COLOR*)VirtualAlloc(0, (x * y + x) * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    int centerX = x / 2;
    int centerY = y / 2;
    double maxDistance = sqrt(centerX * centerX + centerY * centerY);
    double hue, saturation, lightness;

    for (int i = 0; i < 10; i++) {
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

        for (int i = 0; i < x * y; ++i) {
            int x2 = i % x;
            int y2 = i / x;

            double distance = sqrt((x2 - centerX) * (x2 - centerX) + (y2 - centerY) * (y2 - centerY));
            double factor = pow(distance / maxDistance, 2);

            RGBtoHSV(data[i].rgb, hue, saturation, lightness);

            hue = fmod(hue + (factor * 360), 360);

            data[i].rgb = HSVtoRGB(hue, saturation, lightness);
        }

        SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        Sleep(10);
    }

    while (true) {
        r = rand() % 2;
        if (r == 0) {
            for (int i = 0; i < 20; i++) {
                StretchBlt(mdc, 10, 10, x - 20, y - 20, hdc, 0, 0, x, y, SRCCOPY);
                GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

                for (int j = 0; j < x * y; ++j) {
                    if (!(sin(1000 * fade(v))))
                        v = rand() % 100;
                    data[j].rgb += v * 4;
                }

                SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
                StretchBlt(hdc, -10, -10, x + 20, y + 20, mdc, 0, 0, x, y, SRCCOPY);
                Sleep(10);
            }
        }

        if (r == 1) {
            RedrawScreen(0);
            Sleep(10);

            BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
            GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

            for (int i = 0; i < x * y; ++i) {
                int x2 = i % x;
                int y2 = i / x;

                double distance = sqrt((x2 - centerX) * (x2 - centerX) + (y2 - centerY) * (y2 - centerY));
                double factor = pow(distance / maxDistance, 2);

                RGBtoHSV(data[i].rgb, hue, saturation, lightness);

                hue = fmod(hue + (factor * 360), 360);

                data[i].rgb = HSVtoRGB(hue, saturation, lightness);
            }

            SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
            BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

            Sleep(10);
        }

        if (rand() % 3 == 0) {
            RedrawScreen(0);
            Sleep(20);
        }
    }
    return 0;
}

DWORD WINAPI colorShift(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int j = 0;

    COLOR* data = (COLOR*)VirtualAlloc(0, (x * y + x) * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    while (true) {
        StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

        j = rand() % 360;

        for (int i = 0; i < x * y; ++i) {
            double hue, saturation, lightness;
            
            RGBtoHSV(data[i].hsv, hue, saturation, lightness);

            int red = (GetRValue(data[i].rgb) + 10) % 256;
            int green = (GetGValue(data[i].rgb) + 10) % 256;
            int blue = (GetBValue(data[i].rgb) + 10) % 256;

            hue = fmod(hue + j, 360);

            data[i].hsv = HSVtoRGB(hue, saturation, lightness);

            //data[i].rgb = RGB(red, green, blue);
        }

        SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
        StretchBlt(hdc, 0, 0, x, y, mdc, 0, 0, x, y, SRCCOPY);

        Sleep(10);

        if (rand() % 3 == 0) {
            //RedrawScreen(0);
            //Sleep(20);
        }
    }
    return 0;
}

#pragma comment(lib, "winmm.lib")

HWAVEOUT wave = 0;

VOID WINAPI Reset() {
    waveOutReset(wave);
    wave = 0;
}

VOID WINAPI bytebeat_1() {
    const int hz = 8000;
    char buffer[hz * 20];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        buffer[i] = (unsigned char)(((sample1) * 0.2) + random() % 127) << random() % 4 >> 6 | i << 16 + i << 2 + ((((i >> 2) - (i >> 9) | i >> 6)) - random() % 360) << random() % 3;
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

VOID WINAPI bytebeat_2() {
    const int hz = 11025;
    char buffer[hz * 60];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        //buffer[i] = (unsigned char)(((sample1 + sample2 + sample3) * 0.2) + random() % 256); //end part
        //buffer[i] = (unsigned char)(((sample1 + sample2 + sample3) * 0.2) + 127) << 3 | i >> 18 << random() % 3; //epilepsy part
        buffer[i] = (rand() % 32 + (sample1 << 2)) + (((i >> 2) * (i >> 9) | i >> 6)) - 1; //2 part;
        //buffer[i] = (((i >> 4) * (i >> 3) | i >> 2) & ((i >> 2) * (i >> 9) | i >> 6)) - 1; //hue + circle + cubes
        //buffer[i] = (unsigned char)(((sample1) * 0.2) + 127) << 6 | i >> 16; //purple
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

VOID WINAPI bytebeat_3() {
    const int hz = 11025;
    char buffer[hz * 10];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        buffer[i] = (unsigned char)(((sample1 + sample2 + sample3) * 0.2) + 127) << 3 | i >> 18 << random() % 3;
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

VOID WINAPI bytebeat_4() {
    const int hz = 11025;
    char buffer[hz * 20];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, 1, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    for (int i = 0; i < buffsize; i++) {
        float t = (float)i / hz;

        buffer[i] = (((i >> 4) * (i >> 3) | i >> 2 & (i >> 30) ^ i >> 15) + ((i >> 2) * (i >> 9) | i >> 6)) - 1;
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(hwo, &hdr, sizeof(hdr));
    waveOutClose(hwo);
}

VOID WINAPI bytebeat_5() {
    const int hz = 8000;
    char buffer[hz * 30];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, 1, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    for (int i = 0; i < buffsize; i++) {
        buffer[i] = (((i >> 8) * (i >> 4) | i >> 3) * 2) - 1;
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(hwo, &hdr, sizeof(hdr));
    waveOutClose(hwo);
}

VOID WINAPI bytebeat_6() {
    const int hz = 8000;
    char buffer[hz * 30];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, 1, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    for (int i = 0; i < buffsize; i++) {
        buffer[i] = (((i >> 7) * (i >> 5) | i >> 3) ^ 4) - 1; //^ 5 to * 5
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(hwo, &hdr, sizeof(hdr));
    waveOutClose(hwo);
}

VOID WINAPI bytebeat_8() {
    const int hz = 11025;
    char buffer[hz * 20];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    for (int i = 0; i < buffsize; i++) {
        float t = (float)i / hz;
        float f1 = 220.0 * sin(2 * 3.14159 * t * (2 + sin(t * 4)));
        float meow = (sin(f1 * 2 * 3.14159) + sin(f1 * 4 * 3.14159) * 0.5) * 0.5;

        buffer[i] = (char)((meow + 1.0) * 127);
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

VOID WINAPI bytebeat_7() {
    const int hz = 11025;
    char buffer[hz * 30];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, 1, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int pattern_length = hz / 2;

    for (int i = 0; i < buffsize; i++) {
        float t = (float)i / hz;
        int segment = (i / pattern_length) % 8;

        float frequency;
        if (segment < 4) {
            frequency = 220.0 * (1 + segment / 3.0f);
        }
        else {
            frequency = 220.0 * (1 + (2 - (segment - 3)) / 3.0f);
        }

        float wave = sin(2 * 3.14159 * t * frequency);
        float meow = (sin(wave * 2 * 3.14159) + sin(wave * 4 * 3.14159) * 0.5) * 0.5;

        buffer[i] = (char)((meow + 10.0) * 127);
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(hwo, &hdr, sizeof(hdr));
    waveOutClose(hwo);
}

VOID WINAPI bytebeat_9() {
    const int hz = 11025;
    char buffer[hz * 60];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        buffer[i] = (unsigned char)(((sample1) * 0.2) + 127) << 6 | i >> 16;
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

VOID WINAPI bytebeat_10() {
    const int hz = 8000;
    char buffer[hz * 60];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        buffer[i] = (unsigned char)((2 * i << 3) + (i << 1 | i >> 5) + (i >> 4) * i + (20) + (2 * i << 3 + (i << 10 | i >> 7) | i + (8)) + (((i >> 2) * (i >> 9) | i >> 6) + sqrt(i << 6 | i >> 16)) + (i >> 5 | (i >> 2) * (i >> 5))) & 255;
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

VOID WINAPI bytebeat_11() {
    const int hz = 11025;
    char buffer[hz * 25];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        buffer[i] = (((i >> 4) * (i >> 3) | i >> 2) & ((i >> 2) * (i >> 9) | i >> 6)) - 1;
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

VOID WINAPI bytebeat_12() {
    const int hz = 11025;
    char buffer[hz * 50];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        buffer[i] = (unsigned char)(((sample1 + sample2 + sample3) * 0.2) + random() % 64);
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}


VOID WINAPI bytebeat_13() {
    const int hz = 11025;
    char buffer[hz * 60];
    const size_t buffsize = sizeof(buffer);

    HWAVEOUT hwo;
    WAVEHDR hdr = { buffer, buffsize, 0, 0, 0, 0, 0, 0 };
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, hz, hz, WAVE_FORMAT_PCM, 8, 0 };

    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    waveOutPrepareHeader(hwo, &hdr, sizeof(hdr));

    int freq1 = 0, freq2 = 0, freq3 = 0, sample1 = 0, sample2 = 0, sample3 = 0;
    for (int i = 0; i < buffsize; i++) {
        if (i % (int)(hz * 0.166) == 0) {
            freq1 = hz / (35800000 / (32 * ((random() % 510) * 100 + 200 + 1)));
            freq2 = hz / (35800000 / (32 * ((random() % 510) * 100 + 250 + 1)));
            freq3 = hz / (35800000 / (32 * ((random() % 510) * 100 + 325 + 1)));
        }

        sample1 = (i % freq1 < freq1 / 3) ? -127 : 127;
        sample2 = (i % freq2 < freq2 / 3) ? -127 : 127;
        sample3 = (i % freq3 < freq3 / 3) ? -127 : 127;

        buffer[i] = (unsigned char)(i * (rand() % 64) ^ 2 * (i << 30 & i >> 18 * i << 24) + (i + (i >> 9 | i >> 13)));
    }

    waveOutWrite(hwo, &hdr, sizeof(hdr));
    waveOutUnprepareHeader(wave, &hdr, sizeof(hdr));
    waveOutClose(wave);
}

DWORD WINAPI brightWaves(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int rx = 0;
    int r = 0;
    double increment = 0.0;

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);

    while (true) {
        rx = rand() % x;

        hbrush = CreateSolidBrush(COLORHSL(increment));

        SelectObject(hdc, hbrush);

        if (rand() % 2 == 0) {
            BitBlt(hdc, rx, 10, 100, y, hdc, rx, 0, 0xCE646Cu);
        }

        if (rand() % 2 == 1) {
            BitBlt(hdc, rx, -10, -100, y, hdc, rx, 0, 0xCE646Cu);
        }

        Sleep(1);

        increment += 1;

        if (rand() % 30 == 0) {
            RedrawScreen(0);
        }
    }
}

DWORD WINAPI wavesBall(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int rx = 0;
    int ry = 0;
    int xp = 0;
    int yp = 0;

    HDC hdc = GetDC(0);

    while (true) {
        rx = rand() % (x - 200);
        ry = rand() % (y - 200);

        for (int i = 300; i > 0; i--) {
            xp = rx + rand() % 11 - 5;
            yp = ry + rand() % 11 - 5;

            BitBlt(hdc, rx, ry, rand() % (1 + i), rand() % (1 + i), hdc, xp, yp, SRCCOPY);
            
            delay(1);
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

DWORD WINAPI Shake(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    double angle = 0.0;
    int x2;
    int y2;

    HDC hdc = GetDC(0);

    while (true) {
        x2 = (int)(sin(angle) * 20);
        y2 = (int)(cos(angle) * 20);

        BitBlt(hdc, x2, y2, x, y, hdc, 0, 0, SRCCOPY);

        Sleep(140);

        angle++;
    }
}

DWORD WINAPI colorCubes(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int pixelSize = 30;
    int j = 0;

    RGBQUAD* data = (RGBQUAD*)VirtualAlloc(0, (x * y + x) * sizeof(RGBQUAD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 50;

    while (true) {
        hdc = GetDC(0);

        StretchBlt(mdc, 0, 0, x, y, hdc, 0, 0, x, y, SRCPAINT);
        GetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);

        for (int yBlock = 0; yBlock < y; yBlock += pixelSize) {
            for (int xBlock = 0; xBlock < x; xBlock += pixelSize) {
                int red = 0, green = 0, blue = 0, count = 0;
                for (int py = 0; py < pixelSize && (yBlock + py) < y; ++py) {
                    for (int px = 0; px < pixelSize && (xBlock + px) < x; ++px) {
                        int index = ((yBlock + py) * x + (xBlock + px));
                        blue += data[index].rgbBlue;
                        green += data[index].rgbGreen;
                        red += data[index].rgbRed;
                        ++count;
                    }
                }
                if (count > 0) {
                    blue /= count;
                    green /= count;
                    red /= count;
                }
                for (int py = 0; py < pixelSize && (yBlock + py) < y; ++py) {
                    for (int px = 0; px < pixelSize && (xBlock + px) < x; ++px) {
                        int index = ((yBlock + py) * x + (xBlock + px));
                        data[index].rgbBlue = blue;
                        data[index].rgbGreen = green;
                        data[index].rgbRed = red;
                    }
                }
            }
        }

        for (int i = 0; i < x * y; i++) {
            data[i].rgbBlue += (GetBValue(data[i].rgbBlue + rand() % 360) % RGB(255, 255, 255));
            data[i].rgbGreen += (GetGValue(data[i].rgbGreen + rand() % 360) % RGB(255, 255, 255));
            data[i].rgbRed += (GetRValue(data[i].rgbRed + rand() % 360) % RGB(255, 255, 255));
        }

        SetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);
        StretchBlt(hdc, 0, 0, x, y, mdc, 0, 0, x, y, SRCCOPY);
        Sleep(50);

        if (rand() % 3 == 0) {
            RedrawScreen(0);
            Sleep(20);
        }

        j = rand() % 255;
    }
    return 0;
}

DWORD WINAPI hueCircle2(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int x2 = 0;
    int y2 = 0;
    int k = 0;

    COLOR* data = (COLOR*)VirtualAlloc(0, (x * y + x) * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    double hue = 0, saturation = 0, lightness = 0;

    while (true) {
        hdc = GetDC(0);

        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

        for (int i = 0; i < x * y; i++) {
            int x2 = i % x;
            int y2 = i / x;

            double distance = sqrt(pow(x2 - k, 2) + pow(y2 - k, 2));

            RGBtoHSV(data[i].hsv, hue, saturation, lightness);

            hue = fmod((distance)+rand() % 60, 360);

            data[i].hsv = HSVtoRGB(hue, saturation, lightness);
        }

        SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        Sleep(10);

        k = rand() % x;

        if (rand() % 3 == 0) {
            //RedrawWindow(0, 0, 0, 0x85);
            //Sleep(10);
        }
    }
    return 0;
}

DWORD WINAPI dizzy1(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    COLOR* data = (COLOR*)VirtualAlloc(0, (x * y + x) * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    while (true) {
        hdc = GetDC(0);

        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

        for (int i = 0; i < x * y; i++) {
            int x2 = i % x;
            int y2 = i / x;

            data[i].rgb += (x2 + y2) ^ (x2 & y2) + (x2 - y2) & (x2 ^ y2) | y2;
        }

        SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        Sleep(30);
    }
    return 0;
}

DWORD WINAPI Squares(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int rx = 0;
    int ry = 0;

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 127;

    while (true) {
        for (int i = 0; i < 4; i++) {
            rx = rand() % (x - 200);
            ry = rand() % (y - 200);

            BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);

            hbrush = CreateSolidBrush(RGB(255, 234, 9));

            SelectObject(mdc, hbrush);
            PatBlt(mdc, rx, ry, 200, 200, PATCOPY);

            GdiAlphaBlend(hdc, 0, 0, x, y, mdc, 0, 0, x, y, blur);

            Sleep(100);
        }

        hbrush = CreateSolidBrush(40);

        SelectObject(hdc, hbrush);

        BitBlt(hdc, 0, 0, x, y, hdc, 0, 0, PATINVERT);

        Sleep(10);
    }

    ReleaseDC(0, hdc);
    return 0;
}

DWORD WINAPI dizzy2(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int increment = 50;

    HDC hdc = GetDC(0);
    POINT point[3];

    while (true) {
        /*
        rect.left = 0;
        rect.top = 0;
        rect.right = x;
        rect.bottom = y;
        */

        for (int i = 0; i < rand() % 10; i++) {
            point[0].x = increment;
            point[0].y = -increment;
            point[1].x = x + increment;
            point[1].y = increment;
            point[2].x = -increment;
            point[2].y = y - increment;

            PlgBlt(hdc, point, hdc, rand() % 30 - 10, rand() % 30 - 10, x, y, 0, 0, 0);

            Sleep(100);
        }
        for (int i = 0; i < rand() % 10; i++) {
            point[0].x = -increment;
            point[0].y = increment;
            point[1].x = x - increment;
            point[1].y = -increment;
            point[2].x = increment;
            point[2].y = y + increment;

            PlgBlt(hdc, point, hdc, rand() % 30 - 10, rand() % 30 - 10, x, y, 0, 0, 0);

            Sleep(100);
        }
    }
}

DWORD WINAPI paintRectangles(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int x2 = 0;
    int y2 = 0;
    int size = 100;

    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    BLENDFUNCTION blur = { 0 };
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 127;

    while (true) {
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);

        BitBlt(mdc, x2, y2, size, size, hdc, x2, y2, WHITENESS);

        GdiAlphaBlend(hdc, 0, 0, x, y, mdc, 0, 0, x, y, blur);

        x2 += size;

        if (x2 >= x) {
            y2 += size;
            x2 = 0;
        }
        if (y2 >= y) {
            x2 = 0;
            y2 = 0;
        }

        Sleep(50);
    }
}

DWORD WINAPI patInvert(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);

    while (true) {
        hbrush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));

        SelectObject(hdc, hbrush);

        BitBlt(hdc, 0, 0, x, y, hdc, 0, 0, PATINVERT);

        Sleep(100);

        if (rand() % 30 == 0) {
            RedrawScreen(0);
        }
    }
}

DWORD WINAPI pixelShift(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int x2 = 0;
    int y2 = 0;
    int j = 240;

    COLOR* data = (COLOR*)VirtualAlloc(0, x * y * sizeof(COLOR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    while (true) {
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(COLOR), data);

        for (int i = 0; i < x * y; i++) {
            x2 = i % x, y2 = i / y;

            data[i].rgb = (data[i].rgb + rand() % 1000) % (RGB(255, 255, 255));
        }

        SetBitmapBits(bmp, x * y * sizeof(COLOR), data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        Sleep(10);
    }
}

DWORD WINAPI invertRectangles(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int x2 = 0;
    int y2 = 0;
    int size = 300;

    HDC hdc = GetDC(0);

    while (true) {
        BitBlt(hdc, x2, y2, size, size, hdc, x2, y2, NOTSRCCOPY);

        x2 += size;

        if (x2 >= x) {
            y2 += size;
            x2 = 0;
        }
        if (y2 >= y) {
            x2 = 0;
            y2 = 0;
        }

        Sleep(50);
    }
}

DWORD WINAPI colorRectangles(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int x2 = 0;
    int y2 = 0;
    int size = 300;
    int color = 0;

    HBRUSH hbrush = 0;
    HDC hdc = GetDC(0);

    while (true) {
        hbrush = CreateSolidBrush(COLORHSL(color));

        SelectObject(hdc, hbrush);
        BitBlt(hdc, x2, y2, size, size, hdc, x2, y2, PATINVERT);

        x2 += size;

        if (x2 >= x) {
            y2 += size;
            x2 = 0;
        }
        if (y2 >= y) {
            x2 = 0;
            y2 = 0;
        }

        Sleep(50);

        color++;
    }
}

DWORD WINAPI cubes2(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, x, y);
    SelectObject(mdc, bmp);

    while (true) {
        StretchBlt(mdc, 10, 10, x - 20, y - 10, hdc, 0, 0, x, y, SRCCOPY);

        StretchBlt(hdc, -10, -10, x + 20, y + 10, mdc, 0, 0, x, y, SRCCOPY);

        Sleep(10);
    }
}

DWORD WINAPI yellow(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int x2 = 0;
    int y2 = 0;
    int j = 240;

    RGBQUAD* data = (RGBQUAD*)VirtualAlloc(0, x * y * sizeof(RGBQUAD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    while (true) {
        BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);

        for (int i = 0; i < x * y; i++) {
            x2 = i % x, y2 = i / y;

            data[i].rgbBlue = (data[i].rgbBlue + rand() % 1000) % (RGB(255, 255, 255));
        }

        SetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);
        BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

        Sleep(10);

        if (rand() % 3 == 0) {
            RedrawScreen(0);
        }
    }
}

DWORD WINAPI lastShader(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int x2 = 0;
    int y2 = 0;
    int j = 240;

    RGBQUAD* data = (RGBQUAD*)VirtualAlloc(0, x * y * sizeof(RGBQUAD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    HDC hdc = GetDC(0);
    HDC mdc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateBitmap(x, y, 1, 32, data);
    SelectObject(mdc, bmp);

    while (true) {
        for (int i = 0; i < 4; i++) {
            BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
            GetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);

            for (int i = 0; i < x * y; i++) {
                x2 = i % x, y2 = i / y;

                data[i].rgbRed = (data[i].rgbRed = (i % x)) % (RGB(255, 255, 255));
            }

            SetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);
            BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

            Sleep(100);
        }

        RedrawScreen(0);
        Sleep(10);

        for (int i = 0; i < 4; i++) {
            BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
            GetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);

            for (int i = 0; i < x * y; i++) {
                x2 = i % x, y2 = i / y;

                data[i].rgbGreen = (data[i].rgbGreen = (i % x)) % (RGB(255, 255, 255));
            }

            SetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);
            BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

            Sleep(100);
        }

        RedrawScreen(0);
        Sleep(10);

        for (int i = 0; i < 4; i++) {
            BitBlt(mdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);
            GetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);

            for (int i = 0; i < x * y; i++) {
                x2 = i % x, y2 = i / y;

                data[i].rgbBlue = (data[i].rgbBlue = (i % x)) % (RGB(255, 255, 255));
            }

            SetBitmapBits(bmp, x * y * sizeof(RGBQUAD), data);
            BitBlt(hdc, 0, 0, x, y, mdc, 0, 0, SRCCOPY);

            Sleep(100);
        }

        RedrawScreen(0);
        Sleep(10);
    }
}

DWORD WINAPI dizzy3(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int increment = 200;

    HDC hdc = GetDC(0);
    POINT point[3];

    while (true) {
        /*
        rect.left = 0;
        rect.top = 0;
        rect.right = x;
        rect.bottom = y;
        */

        for (int i = 0; i < rand() % 10; i++) {
            point[0].x = increment;
            point[0].y = -increment;
            point[1].x = x + increment;
            point[1].y = increment;
            point[2].x = -increment;
            point[2].y = y - increment;

            PlgBlt(hdc, point, hdc, 0, 0, x, y, 0, 0, 0);

            Sleep(100);
        }
        for (int i = 0; i < rand() % 10; i++) {
            point[0].x = -increment;
            point[0].y = increment;
            point[1].x = x - increment;
            point[1].y = -increment;
            point[2].x = increment;
            point[2].y = y + increment;

            PlgBlt(hdc, point, hdc, 0, 0, x, y, 0, 0, 0);

            Sleep(100);
        }
    }
}

DWORD WINAPI lastWaves(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int ry = 0;

    HDC hdc = GetDC(0);

    while (true) {
        ry = rand() % y;

        BitBlt(hdc, 0, ry, x, 30, hdc, 50, ry, SRCCOPY);

        Sleep(100);
    }
    return 0;
}

DWORD WINAPI lastSineWaves(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int ry = 0;
    double angle = 0.0;

    HDC hdc = GetDC(0);

    while (true) {
        for (float i = 0; i < x + y; i += 20) {
            int a = sin(angle) * 20;
            BitBlt(hdc, 0, i, x, 20, hdc, a, i, SRCCOPY);
            angle += 3.14159 / 40;

            Sleep(100);
        }
    }
    return 0;
}

DWORD WINAPI lastBrightWaves(LPVOID lpParam) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);
    int rx = 0;
    int r = 0;

    HDC hdc = GetDC(0);

    while (true) {
        rx = rand() % x;

        if (rand() % 2 == 0) {
            BitBlt(hdc, rx, 10, 100, y, hdc, rx, 0, SRCPAINT);
        }

        if (rand() % 2 == 1) {
            BitBlt(hdc, rx, -10, -100, y, hdc, rx, 0, SRCPAINT);
        }

        Sleep(150);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    bytebeat_1();
    //fileno;
    HANDLE payload1 = CreateThread(0, 0, firstPayload, 0, 0, 0);
    Sleep(20000);
    KillPayload(payload1);
    RedrawScreen(0);

    bytebeat_2();
    //HANDLE payload2a = CreateThread(0, 0, secondPayload, 0, 0, 0);
    HANDLE payload2a = CreateThread(0, 0, alphaBlur, 0, 0, 0);
    HANDLE payload2b = CreateThread(0, 0, hueNoShift, 0, 0, 0);
    HANDLE payload2c = CreateThread(0, 0, SquaresRGB, 0, 0, 0);
    Sleep(10000);
    HANDLE payload2d = CreateThread(0, 0, xWaves, 0, 0, 0);
    Sleep(20000);
    KillPayload(payload2d);
    HANDLE payload2e = CreateThread(0, 0, waves, 0, 0, 0);
    Sleep(30000);
    KillPayload(payload2a);
    KillPayload(payload2b);
    KillPayload(payload2c);
    KillPayload(payload2e);
    RedrawScreen(0);

    bytebeat_3();
    HANDLE payload3a = CreateThread(0, 0, touhouPayload, 0, 0, 0);
    Sleep(10000);
    KillPayload(payload3a);
    RedrawScreen(0);

    bytebeat_4();
    HANDLE payload4a = CreateThread(0, 0, thirdPayload, 0, 0, 0);
    HANDLE payload4b = CreateThread(0, 0, shapesPayload, 0, 0, 0);
    HANDLE payload4c = CreateThread(0, 0, colorShift, 0, 0, 0);
    Sleep(20000);
    KillPayload(payload4a);
    KillPayload(payload4b);
    KillPayload(payload4c);
    RedrawScreen(0);

    bytebeat_5();
    HANDLE payload5a = CreateThread(0, 0, waves, 0, 0, 0);
    Sleep(5000);
    HANDLE payload5b = CreateThread(0, 0, invert, 0, 0, 0);
    HANDLE payload5c = CreateThread(0, 0, circlePayload, 0, 0, 0);
    Sleep(25000);
    KillPayload(payload5a);
    SuspendThread(payload5b);
    KillPayload(payload5c);
    RedrawScreen(0);

    bytebeat_6();
    HANDLE payload6a = CreateThread(0, 0, cube, 0, 0, 0);
    HANDLE payload6b = CreateThread(0, 0, highWaves, 0, 0, 0);
    Sleep(30000);
    SuspendThread(payload6a);
    KillPayload(payload6b);
    RedrawScreen(0);

    bytebeat_7();
    HANDLE payload7a = CreateThread(0, 0, brightWaves, 0, 0, 0);
    HANDLE payload7c = CreateThread(0, 0, hueNoShift, 0, 0, 0);
    Sleep(15000);
    ResumeThread(payload6a);
    Sleep(20000);
    HANDLE payload7b = CreateThread(0, 0, cubes, 0, 0, 0);
    Sleep(5000);
    SuspendThread(payload6a);
    KillPayload(payload7a);
    KillPayload(payload7b);
    KillPayload(payload7c);
    RedrawScreen(0);

    bytebeat_8();
    HANDLE payload8a = CreateThread(0, 0, highWaves, 0, 0, 0);
    Sleep(10000);
    KillPayload(payload8a);
    HANDLE payload8b = CreateThread(0, 0, stretch, 0, 0, 0);
    ResumeThread(payload6a);
    Sleep(10000);
    SuspendThread(payload6a);
    KillPayload(payload8b);
    RedrawScreen(0);

    bytebeat_9();
    HANDLE payload9f = CreateThread(0, 0, invert, 0, 0, 0);
    HANDLE payload9a = CreateThread(0, 0, wavesBall, 0, 0, 0);
    Sleep(5000);
    HANDLE payload9b = CreateThread(0, 0, Shake, 0, 0, 0);
    Sleep(5000);
    KillPayload(payload9a);
    HANDLE payload9c = CreateThread(0, 0, colorShift, 0, 0, 0);
    Sleep(5000);
    KillPayload(payload9c);
    HANDLE payload9d = CreateThread(0, 0, hueCircle2, 0, 0, 0);
    Sleep(15000);
    HANDLE payload9e = CreateThread(0, 0, firstPayload, 0, 0, 0);
    Sleep(10000);
    HANDLE payload9g = CreateThread(0, 0, colorCubes, 0, 0, 0);
    Sleep(20000);
    KillPayload(payload9f);
    KillPayload(payload9b);
    KillPayload(payload9d);
    KillPayload(payload9e);
    RedrawScreen(0);

    bytebeat_10();
    HANDLE payload10a = CreateThread(0, 0, thirdPayload, 0, 0, 0);
    Sleep(4000);
    HANDLE payload10b = CreateThread(0, 0, colorCubes, 0, 0, 0);
    Sleep(1000);
    HANDLE payload10c = CreateThread(0, 0, stretch, 0, 0, 0);
    Sleep(2000);
    KillPayload(payload10c);
    HANDLE payload10d = CreateThread(0, 0, firstPayload, 0, 0, 0);
    Sleep(3000);
    HANDLE payload10e = CreateThread(0, 0, brightWaves, 0, 0, 0);
    Sleep(2000);
    KillPayload(payload10b);
    KillPayload(payload10e);
    HANDLE payload10j = CreateThread(0, 0, patInvert, 0, 0, 0);
    Sleep(18000);
    KillPayload(payload10j);
    KillPayload(payload10a);
    HANDLE payload10f = CreateThread(0, 0, Squares, 0, 0, 0);
    HANDLE payload10g = CreateThread(0, 0, dizzy2, 0, 0, 0);
    Sleep(30000);
    bytebeat_10();
    KillPayload(payload10d);
    KillPayload(payload10f);
    KillPayload(payload10g);
    Sleep(10000);
    HANDLE payload10h = CreateThread(0, 0, paintRectangles, 0, 0, 0);
    HANDLE payload10i = CreateThread(0, 0, thirdPayload, 0, 0, 0);
    Sleep(10000);
    HANDLE payload10k = CreateThread(0, 0, firstPayload, 0, 0, 0);
    HANDLE payload10l = CreateThread(0, 0, thirdPayload, 0, 0, 0);
    Sleep(5000);
    HANDLE payload10m = CreateThread(0, 0, patInvert, 0, 0, 0);
    Sleep(5000);
    HANDLE payload10n = CreateThread(0, 0, dizzy1, 0, 0, 0);
    Sleep(5000);
    HANDLE payload10o = CreateThread(0, 0, pixelShift, 0, 0, 0);
    Sleep(25000);
    KillPayload(payload10h);
    KillPayload(payload10i);
    KillPayload(payload10k);
    KillPayload(payload10l);
    KillPayload(payload10m);
    KillPayload(payload10n);
    KillPayload(payload10o);
    RedrawScreen(0);

    bytebeat_11();
    HANDLE payload11a = CreateThread(0, 0, cubes, 0, 0, 0);
    Sleep(25000);
    KillPayload(payload11a);
    RedrawScreen(0);

    bytebeat_12();
    HANDLE payload12a = CreateThread(0, 0, invert, 0, 0, 0);
    HANDLE payload12b = CreateThread(0, 0, colorRectangles, 0, 0, 0);
    HANDLE payload12c = CreateThread(0, 0, xWaves, 0, 0, 0);
    HANDLE payload12d = CreateThread(0, 0, cubes2, 0, 0, 0);
    Sleep(10000);
    KillPayload(payload12b);
    HANDLE payload12e = CreateThread(0, 0, invertRectangles, 0, 0, 0);
    HANDLE payload12f = CreateThread(0, 0, colorShift, 0, 0, 0);
    HANDLE payload12g = CreateThread(0, 0, yellow, 0, 0, 0);
    HANDLE payload12h = CreateThread(0, 0, Squares, 0, 0, 0);
    Sleep(25000);
    KillPayload(payload12a);
    KillPayload(payload12b);
    KillPayload(payload12d);
    KillPayload(payload12e);
    KillPayload(payload12g);
    KillPayload(payload12h);
    Sleep(15000);
    KillPayload(payload12c);
    KillPayload(payload12f);
    RedrawScreen(0);

    bytebeat_13();
    HANDLE payload13a = CreateThread(0, 0, lastShader, 0, 0, 0);
    HANDLE payload13b = CreateThread(0, 0, lastWaves, 0, 0, 0);
    HANDLE payload13c = CreateThread(0, 0, dizzy3, 0, 0, 0);
    Sleep(20000);
    KillPayload(payload13b);
    KillPayload(payload13c);
    HANDLE payload13d = CreateThread(0, 0, lastSineWaves, 0, 0, 0);
    HANDLE payload13e = CreateThread(0, 0, lastBrightWaves, 0, 0, 0);
    Sleep(10000);
    KillPayload(payload13a);
    Sleep(30000);
    ExitProcess(0);
}