#ifndef ORTPROXY_H
#define ORTPROXY_H

#include <stdint.h>

#include "shared_global.h"

#ifdef __cplusplus
COM_EXTERN_C_BEGIN
#endif

#ifdef _WIN32
// Define ORT_DLL_IMPORT if your program is dynamically linked to Ort.
// dllexport is not used, we use a .def file.
#    ifdef ORT_DLL_IMPORT
#        define ORT_EXPORT __declspec(dllimport)
#    else
#        define ORT_EXPORT
#    endif
#    define ORT_API_CALL _stdcall
#    define ORT_MUST_USE_RESULT
#    define ORTCHAR_T wchar_t
#else
// To make symbols visible on macOS/iOS
#    ifdef __APPLE__
#        define ORT_EXPORT __attribute__((visibility("default")))
#    else
#        define ORT_EXPORT
#    endif
#    define ORT_API_CALL
#    define ORT_MUST_USE_RESULT __attribute__((warn_unused_result))
#    define ORTCHAR_T           char
#endif

#ifdef __cplusplus
// For any compiler with C++11 support, MSVC 2015 and greater, or Clang version supporting noexcept.
// Such complex condition is needed because compilers set __cplusplus value differently.
#    ifndef __has_feature
#        define __has_feature(x) 0
#    endif
#    if ((__cplusplus >= 201103L) || (_MSC_VER >= 1900) ||                                         \
         (defined(__has_feature) && __has_feature(cxx_noexcept)))
#        define NO_EXCEPTION noexcept
#    else
#        define NO_EXCEPTION throw()
#    endif
#else
#    define NO_EXCEPTION
#endif

typedef struct OrtSessionOptions OrtSessionOptions;

COM_DECL_EXPORT bool ortproxy_init(const char *path);

COM_DECL_EXPORT void *ORT_API_CALL OrtGetApiBase() NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_CPU(
    OrtSessionOptions *options, int use_arena) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_Nnapi(
    OrtSessionOptions *options, uint32_t nnapi_flags) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_Nnapi(
    OrtSessionOptions *options, uint32_t nnapi_flags) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_CoreML(
    OrtSessionOptions *options, uint32_t coreml_flags) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_Dnnl(
    OrtSessionOptions *options, int use_arena) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_CUDA(
    OrtSessionOptions *options, int device_id) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_ROCM(
    OrtSessionOptions *options, int device_id) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_DML(
    OrtSessionOptions *options, int device_id) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_OpenVINO(
    OrtSessionOptions *options, const char *device_id) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_Tensorrt(
    OrtSessionOptions *options, int device_id) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_MIGraphX(
    OrtSessionOptions *options, int device_id) NO_EXCEPTION;

COM_DECL_EXPORT void *ORT_API_CALL OrtSessionOptionsAppendExecutionProvider_Tvm(
    OrtSessionOptions *options, const char *settings) NO_EXCEPTION;

#ifdef __cplusplus
COM_EXTERN_C_END
#endif

#endif // ORTPROXY_H
