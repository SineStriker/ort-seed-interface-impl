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
}