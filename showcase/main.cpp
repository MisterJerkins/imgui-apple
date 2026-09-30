// CupertinoUi showcase: a macOS desktop on Windows and DirectX 11 with every widget and the live examples.

#include "Cupertino.h"
#include "Examples.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <d3d11.h>
#include <windows.h>

#include <ctime>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static ID3D11Device* device = nullptr;
static ID3D11DeviceContext* deviceContext = nullptr;
static IDXGISwapChain* swapChain = nullptr;
static ID3D11RenderTargetView* renderTarget = nullptr;
static bool swapChainOccluded = false;
static UINT resizeWidth = 0;
static UINT resizeHeight = 0;
static RECT windowedRect = {};

static void CreateRenderTarget() {
    ID3D11Texture2D* back_buffer = nullptr;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device->CreateRenderTargetView(back_buffer, nullptr, &renderTarget);
    back_buffer->Release();
}

static void DestroyRenderTarget() {
    if (renderTarget) {
        renderTarget->Release();
        renderTarget = nullptr;
    }
}

static bool CreateDevice(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hwnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL level;
    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &desc, &swapChain, &device, &level, &deviceContext);
    // Without a GPU, WARP renders in software.
    if (result == DXGI_ERROR_UNSUPPORTED)
        result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &desc, &swapChain, &device, &level, &deviceContext);
    if (result != S_OK)
        return false;
    CreateRenderTarget();
    return true;
}

static void DestroyDevice() {
    DestroyRenderTarget();
    if (swapChain) {
        swapChain->Release();
        swapChain = nullptr;
    }
    if (deviceContext) {
        deviceContext->Release();
        deviceContext = nullptr;
    }
    if (device) {
        device->Release();
        device = nullptr;
    }
}

static LRESULT WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;
    switch (msg) {
        case WM_SIZE:
            if (wparam != SIZE_MINIMIZED) {
                resizeWidth = LOWORD(lparam);
                resizeHeight = HIWORD(lparam);
            }
            return 0;
        case WM_DPICHANGED: {
            const RECT* suggested = reinterpret_cast<const RECT*>(lparam);
            SetWindowPos(hwnd, nullptr, suggested->left, suggested->top, suggested->right - suggested->left, suggested->bottom - suggested->top, SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }
        case WM_SYSCOMMAND:
            if ((wparam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

// Full screen covers the monitor without a frame, like the desktop it shows.
static void SetFullScreen(HWND hwnd, bool full_screen) {
    if (full_screen) {
        GetWindowRect(hwnd, &windowedRect);
        MONITORINFO monitor = {sizeof(monitor)};
        GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &monitor);
        const RECT& screen = monitor.rcMonitor;
        SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(hwnd, HWND_TOP, screen.left, screen.top, screen.right - screen.left, screen.bottom - screen.top, SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
    } else {
        SetWindowLongPtrW(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        SetWindowPos(hwnd, nullptr, windowedRect.left, windowedRect.top, windowedRect.right - windowedRect.left, windowedRect.bottom - windowedRect.top, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }
}

struct Desktop {
    bool showcase = true;
    bool settings = false;
    bool activityMonitor = false;
    bool tabbedSettings = false;
    Examples::AppearancePreferences appearance;
    float zoom = 1.0f;
    bool fullScreen = true;
    bool quit = false;
};

static void MenuBar(Desktop& desktop, const std::tm& local) {
    using namespace Cupertino;
    char clock[32];
    std::strftime(clock, sizeof(clock), "%a %b %e  %H:%M", &local);
    Cupertino::MenuBar({.status = clock}, [&] {
        Menu("##apple", {.symbol = Symbols::AppleLogo}, [&] {
            if (Button("System Settings\xE2\x80\xA6"))
                desktop.settings = true;
            Divider();
            if (Button("Quit Showcase", {.shortcut = {"q"}}))
                desktop.quit = true;
        });
        Menu("Showcase", [&] {
            if (Button("Show Showcase", {.shortcut = {"1"}}))
                desktop.showcase = true;
            Divider();
            if (Button("Quit Showcase", {.shortcut = {"q"}}))
                desktop.quit = true;
        });
        Menu("View", [&] {
            int theme = int(desktop.appearance.theme);
            if (Picker("Appearance", &theme, {"Light", "Dark", "Auto"}, {.style = PickerStyle::Inline}))
                desktop.appearance.theme = Examples::ThemeChoice(theme);
            static const char* const Accents[] = {"Multicolor", "Blue", "Purple", "Pink", "Red", "Orange", "Yellow", "Green", "Graphite"};
            int accent = int(desktop.appearance.accent);
            if (Picker("Accent Color", &accent, Accents))
                desktop.appearance.accent = AccentColor(accent);
            static const float Zooms[] = {0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
            int zoom = 1;
            for (int i = 0; i < IM_ARRAYSIZE(Zooms); ++i) {
                if (Zooms[i] == desktop.zoom)
                    zoom = i;
            }
            if (Picker("Zoom", &zoom, {"75%", "100%", "125%", "150%", "175%", "200%"}))
                desktop.zoom = Zooms[zoom];
            Toggle("Wallpaper Tinting", &desktop.appearance.wallpaperTinting);
            Divider();
            Toggle("Full Screen", &desktop.fullScreen);
        });
        Menu("Window", [&] {
            Toggle("Showcase", &desktop.showcase);
            Divider();
            Toggle("System Settings", &desktop.settings);
            Toggle("Activity Monitor", &desktop.activityMonitor);
            Toggle("Settings with Tabs", &desktop.tabbedSettings);
        });
    });
}

static void Wallpaper(bool dark) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImRect rect(viewport->Pos, viewport->Pos + viewport->Size);
    const Cupertino::Rgba top = dark ? Cupertino::Rgba::Hex(0x1C2536) : Cupertino::Rgba::Hex(0xCFDDF2);
    const Cupertino::Rgba bottom = dark ? Cupertino::Rgba::Hex(0x2C2238) : Cupertino::Rgba::Hex(0xEEDDE8);
    Cupertino::Draw::FillVerticalGradient(ImGui::GetBackgroundDrawList(), rect, Cupertino::CornerRadii(0.0f), top, bottom);
}

// The wallpaper's blue and violet as they tint window materials; white is no tint.
static Cupertino::Rgba WallpaperTint(bool dark) {
    return dark ? Cupertino::Rgba(0.96f, 0.95f, 1.05f) : Cupertino::Rgba(0.985f, 0.978f, 0.995f);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    ImGui_ImplWin32_EnableDpiAwareness();
    const float scale = ImGui_ImplWin32_GetDpiScaleForMonitor(MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));
    const WNDCLASSEXW window_class = {sizeof(window_class), CS_CLASSDC, WindowProc, 0, 0, instance, nullptr, nullptr, nullptr, nullptr, L"CupertinoUiShowcase", nullptr};
    RegisterClassExW(&window_class);
    HWND hwnd = CreateWindowW(window_class.lpszClassName, L"CupertinoUi Showcase", WS_OVERLAPPEDWINDOW, 100, 100, int(1440 * scale), int(900 * scale), nullptr, nullptr, instance, nullptr);
    if (!CreateDevice(hwnd)) {
        DestroyDevice();
        UnregisterClassW(window_class.lpszClassName, instance);
        return 1;
    }
    Desktop desktop;
    SetFullScreen(hwnd, desktop.fullScreen);
    bool full_screen = desktop.fullScreen;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, deviceContext);
    if (!Cupertino::Initialize({})) {
        MessageBoxW(hwnd, L"The library was built without its fonts. Download SF Pro and SF Mono from developer.apple.com/fonts, put the files in reference/fonts and run tools/bake.py, then build again.", L"CupertinoUi", MB_ICONERROR);
        return 2;
    }

    while (!desktop.quit) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                desktop.quit = true;
        }
        if (desktop.quit)
            break;
        if (desktop.fullScreen != full_screen) {
            full_screen = desktop.fullScreen;
            SetFullScreen(hwnd, full_screen);
        }
        if (swapChainOccluded && swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            Sleep(10);
            continue;
        }
        swapChainOccluded = false;
        if (resizeWidth != 0 && resizeHeight != 0) {
            DestroyRenderTarget();
            swapChain->ResizeBuffers(0, resizeWidth, resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            resizeWidth = resizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        const std::time_t now = std::time(nullptr);
        std::tm local = {};
        localtime_s(&local, &now);
        // Auto is light by day and dark from seven in the evening, as macOS switches it.
        const Examples::ThemeChoice theme = desktop.appearance.theme;
        const bool night = local.tm_hour >= 19 || local.tm_hour < 7;
        Cupertino::EnvironmentValues& environment = Cupertino::Environment();
        environment.displayScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd);
        environment.zoom = desktop.zoom;
        environment.appearance = theme == Examples::ThemeChoice::Dark || (theme == Examples::ThemeChoice::Auto && night) ? Cupertino::Appearance::Dark : Cupertino::Appearance::Light;
        environment.accent = desktop.appearance.accent;
        environment.wallpaperTint = desktop.appearance.wallpaperTinting ? WallpaperTint(environment.IsDark()) : Cupertino::Rgba(1.0f, 1.0f, 1.0f);

        Wallpaper(environment.IsDark());
        MenuBar(desktop, local);
        if (desktop.showcase)
            Cupertino::ShowShowcase(&desktop.showcase, {.position = ImVec2(60.0f, 60.0f)});
        if (desktop.settings)
            Examples::SystemSettings(&desktop.settings, {.appearance = &desktop.appearance});
        if (desktop.activityMonitor)
            Examples::ActivityMonitor(&desktop.activityMonitor);
        if (desktop.tabbedSettings)
            Examples::TabbedSettings(&desktop.tabbedSettings);
        Cupertino::EndFrame();
        ImGui::Render();

        const float clear[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
        deviceContext->ClearRenderTargetView(renderTarget, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swapChainOccluded = swapChain->Present(1, 0) == DXGI_STATUS_OCCLUDED;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    DestroyDevice();
    DestroyWindow(hwnd);
    UnregisterClassW(window_class.lpszClassName, instance);
    return 0;
}
