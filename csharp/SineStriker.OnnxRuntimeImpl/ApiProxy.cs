using System.Text;

namespace SineStriker.OnnxRuntimeImpl;

public static class ApiProxy
{
    /// <summary>
    /// Initialize proxy library
    /// </summary>
    /// <param name="path">Real OnnxRuntime library directory</param>
    /// <param name="relativeToDll">Evaluated relative to library or executable directory if `path` is relative</param>
    public static void Init(string path, bool relativeToDll)
    {
        if (!NativeMethod.ortproxy_init(
                System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(Encoding.UTF8.GetBytes(path), 0),
                relativeToDll
            ))
        {
            throw new Exception("OnnxRuntimeImpl.ApiProxy initialization failed!");
        }
    }
}