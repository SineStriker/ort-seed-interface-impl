namespace OrtSeedImpl;

public static class Library
{
    public static void SetRandomSeed(Int64 seed)
    {
        NativeMethod.NativeTrainingMethods.OrtSetSeed((IntPtr)seed);
    }

    public static Int64 GetRandomSeed()
    {
        NativeMethod.NativeTrainingMethods.OrtGetSeed(out var seed);
        return (Int64)seed;
    }

    private enum OpenVPIRequestType
    {
        SetHint = 0,
        GetHint,
        SetSeed,
        GetSeed,
        RemoveSeed,
    };

    public static void SetHint(Int64 hint)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.SetHint,
            IntPtr.Zero,
            (IntPtr)hint,
            out _
        );
    }

    public static Int64 GetHint()
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.GetHint,
            IntPtr.Zero,
            IntPtr.Zero,
            out var res
        );
        return (Int64)res;
    }

    public static void SetSeed(Int64 key, Int64 value)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.SetSeed,
            (IntPtr)key,
            (IntPtr)value,
            out _
        );
    }

    public static Int64 GetSeed(Int64 key, Int64 defaultValue = 0)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.GetSeed,
            (IntPtr)key,
            (IntPtr)defaultValue,
            out var res
        );
        return (Int64)res;
    }

    public static void RemoveSeed(Int64 key)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.RemoveSeed,
            (IntPtr)key,
            IntPtr.Zero,
            out _
        );
    }
}