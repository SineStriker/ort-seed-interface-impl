#include "ortproxy.h"

#include <Shlwapi.h>
#include <Windows.h>

#include <iomanip>
#include <iostream>

#ifdef CONFIG_DISABLE_DEBUG
#    define COUT                                                                                   \
        while (0)                                                                                  \
        std::cout
#    define WCOUT                                                                                  \
        while (0)                                                                                  \
        std::wcout
#else
#    define COUT  std::cout
#    define WCOUT std::wcout
#endif

struct OrtSessionOptions;

typedef void *(*Func0)();
typedef void *(*Func1)(OrtSessionOptions *, int);
typedef void *(*Func2)(OrtSessionOptions *, uint32_t);
typedef void *(*Func3)(OrtSessionOptions *, const char *);

static void *dummy_Func0() {
    return nullptr;
};

static void *dummy_Func1(OrtSessionOptions *, int) {
    return nullptr;
};

static void *dummy_Func2(OrtSessionOptions *, uint32_t) {
    return nullptr;
};

static void *dummy_Func3(OrtSessionOptions *, const char *) {
    return nullptr;
};

#define CREATE_DELEGATE(FUNC, NAME)                                                                \
    struct D##NAME {                                                                               \
        const char *name = #NAME;                                                                  \
        FUNC func = dummy_##FUNC;                                                                  \
    };                                                                                             \
    D##NAME NAME;

struct dylib {
    // Func0 OrtGetApiBase;
    CREATE_DELEGATE(Func0, OrtGetApiBase)
    CREATE_DELEGATE(Func1, OrtSessionOptionsAppendExecutionProvider_CPU);
    CREATE_DELEGATE(Func2, OrtSessionOptionsAppendExecutionProvider_Nnapi);
    CREATE_DELEGATE(Func2, OrtSessionOptionsAppendExecutionProvider_CoreML);
    CREATE_DELEGATE(Func1, OrtSessionOptionsAppendExecutionProvider_Dnnl);
    CREATE_DELEGATE(Func1, OrtSessionOptionsAppendExecutionProvider_CUDA);
    CREATE_DELEGATE(Func1, OrtSessionOptionsAppendExecutionProvider_ROCM);
    CREATE_DELEGATE(Func1, OrtSessionOptionsAppendExecutionProvider_DML);
    CREATE_DELEGATE(Func3, OrtSessionOptionsAppendExecutionProvider_OpenVINO);
    CREATE_DELEGATE(Func1, OrtSessionOptionsAppendExecutionProvider_Tensorrt);
    CREATE_DELEGATE(Func1, OrtSessionOptionsAppendExecutionProvider_MIGraphX);
    CREATE_DELEGATE(Func3, OrtSessionOptionsAppendExecutionProvider_Tvm);
    HMODULE hDLL = nullptr;

    ~dylib() {
        if (hDLL) {
            ::FreeLibrary(hDLL);
        }
    }
};

static dylib *g_lib = nullptr;

static wchar_t Error_Title[] = TO_UNICODE("Fatal Error");

static wchar_t Module_Dir[MAX_PATH] = {0};

static wchar_t Module_Name[MAX_PATH] = {0};

static void MsgBoxError(const wchar_t *text) {
    ::MessageBoxW(nullptr, text, (*Module_Name) ? Module_Name : Error_Title,
                  MB_OK
#ifdef CONFIG_WIN32_MSGBOX_TOPMOST
                      | MB_TOPMOST
#endif
                      | MB_SETFOREGROUND | MB_ICONERROR);
}

static void AddPath(const wchar_t *path) {
    auto sz = ::GetEnvironmentVariableW(L"Path", nullptr, 0);
    auto buf = new wchar_t[sz + 1];
    ::wmemset(buf, 0, sz + 1);
    ::GetEnvironmentVariableW(L"Path", buf, sz);

    std::wstring str(buf);
    delete[] buf;

    if (str.size() > 0 && str.back() != L';') {
        str += L';';
    }
    str += path;
    ::SetEnvironmentVariableW(L"Path", str.data());
}

std::wstring WinGetLastErrorString(int *errNum = nullptr) {
    // Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::wstring(); // No error message has been recorded
    }

    if (errNum) {
        *errNum = errorMessageID;
    }

    LPWSTR messageBuffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us
    // (because we don't yet know how long the message string will be).
    size_t size = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                       FORMAT_MESSAGE_IGNORE_INSERTS,
                                   NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                   (LPWSTR) &messageBuffer, 0, NULL);

    // Copy the error message into a std::string.
    std::wstring message(messageBuffer, size);

    // Free the Win32's string's buffer.
    ::LocalFree(messageBuffer);

    return message;
}

wchar_t *char_to_wchar(const char *src_char, size_t cp = CP_ACP) {
    auto src_len = strlen(src_char);
    int len = MultiByteToWideChar(cp, 0, src_char, src_len, NULL, 0);
    auto buf = new wchar_t[len + 1];
    MultiByteToWideChar(cp, 0, src_char, src_len, buf, len);
    buf[len] = '\0';
    return buf;
}

char *wchar_to_char(const wchar_t *src_wchar, size_t cp = CP_ACP) {
    auto src_len = wcslen(src_wchar);
    int len = WideCharToMultiByte(cp, 0, src_wchar, src_len, 0, 0, NULL, NULL);
    char *buf = new char[len + 1];
    WideCharToMultiByte(cp, 0, src_wchar, src_len, buf, len, NULL, NULL);
    buf[len] = '\0';
    return buf;
}

bool ortproxy_init(const char *path) {
    // Get module filename
    std::wstring wstr;
    wchar_t buf[MAX_PATH + 1] = {0};
    if (::GetModuleFileNameW(NULL, buf, MAX_PATH) != 0) {
        wstr = buf;
    } else {
        MsgBoxError(TO_UNICODE("Failed to get module path!"));
        return false;
    }

    // Get executable directory
    size_t idx = wstr.find_last_of(L"\\");
    if (idx == std::wstring::npos) {
        MsgBoxError(TO_UNICODE("Bad file path!"));
        return false;
    }
    std::wstring exeDir = wstr.substr(0, idx);
    ::WSTRCPY(Module_Dir, exeDir.data());

    // Get executable
    std::wstring name = wstr.substr(idx + 1);
    idx = name.find_last_of(L".");
    if (idx != std::wstring::npos && idx > 0) {
        name = name.substr(0, idx);
    }
    ::WSTRCPY(Module_Name, name.data());

    // Get library path
    auto path_w = char_to_wchar(path, CP_UTF8);
    std::wstring libdir = path_w;
    delete[] path_w;
    for (auto &ch : libdir) {
        if (ch == L'/')
            ch = L'\\';
    }

    // Determine path is relative or absolute
    if (::PathIsRelativeW(libdir.data())) {
        libdir = exeDir + L"\\" + libdir;
    }

    const auto infoWidth = std::setw(30);

    WCOUT << std::left << infoWidth << "[OrtProxy] SetDllDirectory " << libdir << std::endl;
    // ::SetDllDirectoryW(libdir.data());
    // ::AddPath(libdir.data());

    std::wstring libPath = libdir + L"\\onnxruntime.dll";
    WCOUT << std::left << infoWidth << "[OrtProxy] LoadLibrary " << libPath << std::endl;

    // Load library
    HINSTANCE hDLL = ::LoadLibraryW(libPath.data());
    if (!hDLL) {
        std::wstring msg = WinGetLastErrorString();
        MsgBoxError(msg.data());
        return false;
    }

    // Load entries
    dylib lib;
    lib.hDLL = hDLL;

    auto getEntry = [&](auto &T, bool required = false) {
        auto tmp = (decltype(T.func)) ::GetProcAddress(hDLL, T.name);
        if (tmp) {
            T.func = tmp;
            COUT << std::left << infoWidth << "[OrtProxy] GetProcAddress " << T.name << " "
                 << std::hex << (intptr_t) T.func << std::endl;
        } else {
            if (required) {
                WCOUT << std::left << infoWidth << "[OrtProxy] Get required entry " << T.name
                      << " failed" << std::endl;
                std::wstring msg = WinGetLastErrorString();
                MsgBoxError(msg.data());
            }
            return false;
        }
        return true;
    };

    if (!getEntry(lib.OrtGetApiBase, true)) {
        return false;
    }
    if (!getEntry(lib.OrtSessionOptionsAppendExecutionProvider_CPU, true)) {
        return false;
    }
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_Nnapi);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_CoreML);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_Dnnl);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_CUDA);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_ROCM);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_DML);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_OpenVINO);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_Tensorrt);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_MIGraphX);
    getEntry(lib.OrtSessionOptionsAppendExecutionProvider_Tvm);

    g_lib = new dylib();
    std::swap(*g_lib, lib);
    return true;
}

void *OrtGetApiBase() NO_EXCEPTION {
    return g_lib->OrtGetApiBase.func();
}

void *OrtSessionOptionsAppendExecutionProvider_CPU(OrtSessionOptions *options,
                                                   int use_arena) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_CPU.func(options, use_arena);
}

void *OrtSessionOptionsAppendExecutionProvider_Nnapi(OrtSessionOptions *options,
                                                     uint32_t nnapi_flags) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_Nnapi.func(options, nnapi_flags);
}

void *OrtSessionOptionsAppendExecutionProvider_CoreML(OrtSessionOptions *options,
                                                      uint32_t coreml_flags) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_CoreML.func(options, coreml_flags);
}

void *OrtSessionOptionsAppendExecutionProvider_Dnnl(OrtSessionOptions *options,
                                                    int use_arena) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_Dnnl.func(options, use_arena);
}

void *OrtSessionOptionsAppendExecutionProvider_CUDA(OrtSessionOptions *options,
                                                    int device_id) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_CUDA.func(options, device_id);
}

void *OrtSessionOptionsAppendExecutionProvider_ROCM(OrtSessionOptions *options,
                                                    int device_id) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_ROCM.func(options, device_id);
}

void *OrtSessionOptionsAppendExecutionProvider_DML(OrtSessionOptions *options,
                                                   int device_id) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_DML.func(options, device_id);
}

void *OrtSessionOptionsAppendExecutionProvider_OpenVINO(OrtSessionOptions *options,
                                                        const char *device_id) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_OpenVINO.func(options, device_id);
}

void *OrtSessionOptionsAppendExecutionProvider_Tensorrt(OrtSessionOptions *options,
                                                        int device_id) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_Tensorrt.func(options, device_id);
}

void *OrtSessionOptionsAppendExecutionProvider_MIGraphX(OrtSessionOptions *options,
                                                        int device_id) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_MIGraphX.func(options, device_id);
}

void *OrtSessionOptionsAppendExecutionProvider_Tvm(OrtSessionOptions *options,
                                                   const char *settings) NO_EXCEPTION {
    return g_lib->OrtSessionOptionsAppendExecutionProvider_Tvm.func(options, settings);
}
