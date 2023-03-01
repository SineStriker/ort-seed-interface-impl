using System.Text;

namespace SineStriker.OnnxRuntimeImpl;

public static class Seed
{
    /// <summary>
    /// Set global random seed (Deprecated)
    /// </summary>
    /// <param name="seed"></param>
    public static void SetRandomSeed(Int64 seed)
    {
        NativeMethod.NativeTrainingMethods.OrtSetSeed((IntPtr)seed);
    }

    /// <summary>
    /// Get global random seed (Deprecated)
    /// </summary>
    /// <returns></returns>
    public static Int64 GetRandomSeed()
    {
        NativeMethod.NativeTrainingMethods.OrtGetSeed(out var seed);
        return (Int64)seed;
    }

    enum OpenVPIRequestType
    {
        SetCurrentSessionId = 0,
        GetCurrentSessionId,
        SetSessionSeed,
        GetSessionSeed,
        SetSessionTaskId,
        GetSessionTaskId,
        RemoveSession,
    };

    /// <summary>
    /// Set the current session id, which is supposed to be positive
    /// Call before creating an InferenceSession and then all random operators will be related to the session id
    /// </summary>
    /// <param name="sessionId"></param>
    public static void SetCurrentSessionId(Int64 sessionId)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.SetCurrentSessionId,
            IntPtr.Zero,
            (IntPtr)sessionId,
            out _
        );
    }

    /// <summary>
    /// Get the current session id
    /// </summary>
    /// <returns></returns>
    public static Int64 GetCurrentSessionId()
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.GetCurrentSessionId,
            IntPtr.Zero,
            IntPtr.Zero,
            out var res
        );
        return (Int64)res;
    }

    /// <summary>
    /// Set the seed of the session corresponding to `sessionId`, which is supposed to be positive
    /// If the session doesn't exist then it will be allocated
    /// </summary>
    /// <param name="sessionId"></param>
    /// <param name="seed"></param>
    public static void SetSessionSeed(Int64 sessionId, Int64 seed)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.SetSessionSeed,
            (IntPtr)sessionId,
            (IntPtr)seed,
            out _
        );
    }

    /// <summary>
    /// Get the seed of the session corresponding to `sessionId`
    /// If the session doesn't exist, the `defaultValue` will be returned
    /// </summary>
    /// <param name="sessionId"></param>
    /// <param name="defaultValue"></param>
    /// <returns></returns>
    public static Int64 GetSessionSeed(Int64 sessionId, Int64 defaultValue = 0)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.GetSessionSeed,
            (IntPtr)sessionId,
            (IntPtr)defaultValue,
            out var res
        );
        return (Int64)res;
    }

    /// <summary>
    /// Set current task id of the session corresponding to `sessionId`, which is supposed to be positive
    /// If the session doesn't exist then it will be allocated
    /// </summary>
    /// <param name="sessionId"></param>
    /// <param name="taskId"></param>
    public static void SetSessionTaskId(Int64 sessionId, Int64 taskId)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.SetSessionTaskId,
            (IntPtr)sessionId,
            (IntPtr)taskId,
            out _
        );
    }

    /// <summary>
    /// Get current task id of the session corresponding to `sessionId`
    /// If the session doesn't exist, the `defaultValue` will be returned
    /// </summary>
    /// <param name="sessionId"></param>
    /// <param name="defaultValue"></param>
    /// <returns></returns>
    public static Int64 GetSessionTaskId(Int64 sessionId, Int64 defaultValue = 0)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.GetSessionTaskId,
            (IntPtr)sessionId,
            (IntPtr)defaultValue,
            out var res
        );
        return (Int64)res;
    }

    /// <summary>
    /// Remove a session corresponding to `sessionId`, this call will be ignored if the session doesn't exist
    /// </summary>
    /// <param name="sessionId"></param>
    public static void RemoveSession(Int64 sessionId)
    {
        NativeMethod.NativeTrainingMethods.AccessOpenVPIRandomSeed(
            (int)OpenVPIRequestType.RemoveSession,
            (IntPtr)sessionId,
            IntPtr.Zero,
            out _
        );
    }
}