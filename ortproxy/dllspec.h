#ifndef DLLSPEC_H
#define DLLSPEC_H

#include <iostream>
#include <stdint.h>

#include "shared_global.h"

#ifdef _WIN32
#    include <Shlwapi.h>
#    include <Windows.h>

using PathChar = wchar_t;
using PathString = std::wstring;
using DllHandle = HMODULE;
using EntryHandle = FARPROC;

#    define STR(s) TO_UNICODE(s)
#    define STRCPY WSTRCPY

static PathChar PathSeparator = L'\\';
#else
#    include <dlfcn.h>
#    include <limits.h>
#    include <string.h>

using PathChar = char;
using PathString = std::string;
using DllHandle = void *;
using EntryHandle = void *;

#    define STR(s) s
#    define STRCPY strcpy

static PathChar PathSeparator = '/';
#endif

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

#if _WIN32
#    define PRINT WCOUT
#else
#    define PRINT COUT
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

#ifdef _WIN32
static wchar_t ErrorTitle[] = L"Fatal Error";

static wchar_t AppDirectory[MAX_PATH + 1] = {0};

static wchar_t AppName[MAX_PATH + 1] = {0};
#else

static char AppDirectory[PATH_MAX + 1] = {0};

static char AppName[NAME_MAX + 1] = {0};

#endif

static void ShowError(const PathString &str) {
#ifdef _WIN32
    ::MessageBoxW(nullptr, str.data(), (*AppName) ? AppName : ErrorTitle,
                  MB_OK
#    ifdef CONFIG_WIN32_MSGBOX_TOPMOST
                      | MB_TOPMOST
#    endif
                      | MB_SETFOREGROUND | MB_ICONERROR);
#else
    fprintf(stderr, "%s\n", str.data());
#endif
}

static PathString GetAppPath() {
#ifdef _WIN32
    PathChar buf[MAX_PATH + 1] = {0};
    if (!::GetModuleFileNameW(NULL, buf, MAX_PATH)) {
        return {};
    }
#else
    PathChar buf[PATH_MAX + 1] = {0};
#    ifdef __APPLE__
// To do ...
#    else
    if (!realpath("/proc/self/exe", buf)) {
        return {};
    }
#    endif
#endif
    PathString path(buf);

    // Get dir
    auto idx = path.find_last_of(PathSeparator);
    if (idx == PathString::npos) {
        return {};
    }
    auto exeDir = path.substr(0, idx);
    STRCPY(AppDirectory, exeDir.data());

    // Get name
    PathString name = path.substr(idx + 1);
    idx = name.find_last_of(STR("."));
    if (idx != PathString::npos && idx > 0) {
        name = name.substr(0, idx);
    }
    STRCPY(AppName, name.data());

    return path;
}

static bool IsRelative(const PathChar *path) {
#if _WIN32
    return ::PathIsRelativeW(path);
#else
    while (path && *path == ' ') {
        path++;
    }
    return !path || *path != '/';
#endif
}

static DllHandle OpenDll(const PathChar *path) {
#ifdef _WIN32
    return ::LoadLibraryW(path);
#else
    return dlopen(path, RTLD_NOW);
#endif
}

static void CloseDll(DllHandle handle) {
#ifdef _WIN32
    ::FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}

static EntryHandle GetEntry(DllHandle handle, const char *name) {
#ifdef _WIN32
    return ::GetProcAddress(handle, name);
#else
    return dlsym(handle, name);
#endif
}

static PathString GetSelfName() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH + 1] = {0};
    HMODULE hm = NULL;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR) &GetSelfName, &hm) == 0) {
        return {};
    }
    if (GetModuleFileNameW(hm, buf, sizeof(buf)) == 0) {
        int ret = GetLastError();
        return {};
    }
#else
    Dl_info dl_info;
    dladdr((void *) GetSelfName, &dl_info);
    auto buf = dl_info.dli_fname;
#endif
    PathString path(buf);

    // Get dir
    auto idx = path.find_last_of(PathSeparator);
    if (idx == PathString::npos) {
        return {};
    }

    // Get name
    return path.substr(idx + 1);
}

struct dylib {
#define CREATE_DELEGATE(FUNC, NAME)                                                                \
    struct D##NAME {                                                                               \
        const char *name = #NAME;                                                                  \
        FUNC func = dummy_##FUNC;                                                                  \
    };                                                                                             \
    D##NAME NAME;

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

#undef CREATE_DELEGATE

    DllHandle hDLL = nullptr;

    ~dylib() {
        if (hDLL) {
            CloseDll(hDLL);
        }
    }
};

static dylib *g_lib = nullptr;

#endif // DLLSPEC_H
