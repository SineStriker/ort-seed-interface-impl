#include "ortproxy.h"

#include <iomanip>
#include <iostream>

#include "dllspec.h"

using std::left;

#ifdef _WIN32
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
#endif

bool ortproxy_init(const char *path) {
    if (GetAppPath().empty()) {
        ShowError(
#if _WIN32
            WinGetLastErrorString()
#else
            STR("Bad file path!")
#endif
        );
        return false;
    }

    PathString libdir;
#ifdef _WIN32
    {
        auto path_w = char_to_wchar(path, CP_UTF8);
        libdir = path_w;
        delete[] path_w;
        for (auto &ch : libdir) {
            if (ch == L'/')
                ch = L'\\';
        }
    }
#else
    libdir = path;
#endif

    // Determine path is relative or absolute
    if (IsRelative(libdir.data())) {
        libdir = PathString(AppDirectory) + PathSeparator + libdir;
    }

    const auto infoWidth = std::setw(30);

#ifdef _WIN32
    // SetDllDirectory
    PRINT << left << infoWidth << "[OrtProxy] SetDllDirectory " << libdir << std::endl;
    ::SetDllDirectoryW(libdir.data());
#endif

    PathString libName = GetSelfName();
    if (libName.empty()) {
        ShowError(
#if _WIN32
            WinGetLastErrorString()
#else
            STR("Bad file path!")
#endif
        );
        return false;
    }
    PathString libPath = libdir + PathSeparator + libName;

    // LoadLibrary
    PRINT << left << infoWidth << "[OrtProxy] LoadLibrary " << libPath << std::endl;
    DllHandle hDLL = OpenDll(libPath.data());

    if (!hDLL) {
        ShowError(
#if _WIN32
            WinGetLastErrorString()
#else
            STR("Failed to open library!")
#endif
        );
        return false;
    }

    // GetProcAddress
    dylib lib;
    lib.hDLL = hDLL;

    auto getEntry = [&](auto &T, bool required = false) {
        auto tmp = (decltype(T.func)) GetEntry(hDLL, T.name);
        if (tmp) {
            T.func = tmp;
            COUT << left << infoWidth << "[OrtProxy] GetProcAddress " << T.name << " " << std::hex
                 << (intptr_t) T.func << std::endl;
        } else {
            if (required) {
                ShowError(
#if _WIN32
                    WinGetLastErrorString()
#else
                    STR("Failed to dlsym!")
#endif
                );
                PRINT << left << infoWidth << "[OrtProxy] Get required entry " << T.name
                      << " failed" << std::endl;
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
