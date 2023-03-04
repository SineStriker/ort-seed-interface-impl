using System.Runtime.InteropServices;
using Microsoft.ML.OnnxRuntime;

namespace SineStriker.OnnxRuntimeImpl;

public static class NativeMethod
{
    public static class NativeLib
    {
#if __ANDROID__
        // define the library name required for android
        internal const string DllName = "libonnxruntime.so";
#elif __IOS__
        // define the library name required for iOS
        internal const string DllName = "__Internal";
#else
        internal const string DllName = "onnxruntime";
#endif
        // TODO: Does macos need special handling or will 'onnxruntime' -> libonnxruntime.dylib?
    }

    [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi)]
    public static extern ref OrtApiBase OrtGetApiBase();

    // NOTE: The order of the APIs in this struct should match exactly that in
    // OrtTrainingApi  (onnxruntime_training_c_api.cc)
    [StructLayout(LayoutKind.Sequential)]
    public struct OrtTrainingApi
    {
        public IntPtr LoadCheckpoint;
        public IntPtr SaveCheckpoint;
        public IntPtr CreateTrainingSession;
        public IntPtr TrainingSessionGetTrainingModelOutputCount;
        public IntPtr TrainingSessionGetEvalModelOutputCount;
        public IntPtr TrainingSessionGetTrainingModelOutputName;
        public IntPtr TrainingSessionGetEvalModelOutputName;
        public IntPtr LazyResetGrad;
        public IntPtr TrainStep;
        public IntPtr EvalStep;
        public IntPtr SetLearningRate;
        public IntPtr GetLearningRate;
        public IntPtr OptimizerStep;
        public IntPtr RegisterLinearLRScheduler;
        public IntPtr SchedulerStep;
        public IntPtr GetParametersSize;
        public IntPtr CopyParametersToBuffer;
        public IntPtr CopyBufferToParameters;
        public IntPtr ReleaseTrainingSession;
        public IntPtr ReleaseCheckpointState;
        public IntPtr ExportModelForInferencing;
        public IntPtr SetSeed;
        public IntPtr TrainingSessionGetTrainingModelInputCount;
        public IntPtr TrainingSessionGetEvalModelInputCount;
        public IntPtr TrainingSessionGetTrainingModelInputName;
        public IntPtr TrainingSessionGetEvalModelInputName;
        public IntPtr GetSeed;
        public IntPtr AccessOpenVPIRandomSeed;
    }

    public static class NativeTrainingMethods
    {
        static OrtApi api_;
        static OrtTrainingApi trainingApi_;
        static IntPtr trainingApiPtr;

        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        public delegate ref OrtApi DOrtGetApi(UInt32 version);

        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        public delegate IntPtr /* OrtTrainingApi* */ DOrtGetTrainingApi(UInt32 version);

        public static DOrtGetTrainingApi OrtGetTrainingApi;

        static NativeTrainingMethods()
        {
            DOrtGetApi OrtGetApi =
                (DOrtGetApi)Marshal.GetDelegateForFunctionPointer(OrtGetApiBase().GetApi,
                    typeof(DOrtGetApi));

            // TODO: Make this save the pointer, and not copy the whole structure across
            api_ = OrtGetApi(13 /*ORT_API_VERSION*/);

            OrtGetTrainingApi =
                (DOrtGetTrainingApi)Marshal.GetDelegateForFunctionPointer(api_.GetTrainingApi,
                    typeof(DOrtGetTrainingApi));
            trainingApiPtr = OrtGetTrainingApi(13 /*ORT_API_VERSION*/);

            if (trainingApiPtr != IntPtr.Zero)
            {
                trainingApi_ = (OrtTrainingApi)Marshal.PtrToStructure(trainingApiPtr, typeof(OrtTrainingApi));
                OrtSetSeed =
                    (DOrtSetSeed)Marshal.GetDelegateForFunctionPointer(trainingApi_.SetSeed,
                        typeof(DOrtSetSeed));
                OrtGetSeed =
                    (DOrtGetSeed)Marshal.GetDelegateForFunctionPointer(trainingApi_.GetSeed,
                        typeof(DOrtGetSeed));
                AccessOpenVPIRandomSeed =
                    (DAccessOpenVPIRandomSeed)Marshal.GetDelegateForFunctionPointer(
                        trainingApi_.AccessOpenVPIRandomSeed,
                        typeof(DAccessOpenVPIRandomSeed));
            }
        }

        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        public delegate void DOrtSetSeed(IntPtr seed);

        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        public delegate void DOrtGetSeed(out IntPtr seed);

        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
        public delegate void DAccessOpenVPIRandomSeed(int type, IntPtr key, IntPtr value, out IntPtr @out);

        public static DOrtSetSeed OrtSetSeed;

        public static DOrtGetSeed OrtGetSeed;

        public static DAccessOpenVPIRandomSeed AccessOpenVPIRandomSeed;
    }

    // OnnxRuntime Proxy
    [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern bool ortproxy_init(IntPtr dev, bool relative_to_dll);
}