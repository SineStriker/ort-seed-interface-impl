#include <iostream>

#include "../ortproxy.h"

typedef enum OrtErrorCode {
    ORT_OK,
    ORT_FAIL,
    ORT_INVALID_ARGUMENT,
    ORT_NO_SUCHFILE,
    ORT_NO_MODEL,
    ORT_ENGINE_ERROR,
    ORT_RUNTIME_EXCEPTION,
    ORT_INVALID_PROTOBUF,
    ORT_MODEL_LOADED,
    ORT_NOT_IMPLEMENTED,
    ORT_INVALID_GRAPH,
    ORT_EP_FAIL,
} OrtErrorCode;

struct OrtStatus {
    OrtErrorCode code;
    char msg[1]; // a null-terminated string
};

#ifdef _WIN32
#    include <Windows.h>
#endif

int main(int argc, char *argv[]) {
    ortproxy_init("eps/dml");

    auto res = (OrtStatus *) OrtSessionOptionsAppendExecutionProvider_DML(nullptr, 0);
    std::cout << (intptr_t) res;

    if (res) {
        std::cout << res->code << " " << res->msg << std::endl;
    }

    return 0;
}