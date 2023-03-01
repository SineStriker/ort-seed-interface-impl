using System.Text;

namespace SineStriker.OnnxRuntimeImpl;

public static class ApiProxy
{
    /// <summary>
    /// Initialize proxy library
    /// </summary>
    /// <param name="path">Real OnnxRuntime library directory, evaluated relative to application directory if relative</param>
    public static void Init(string path)
    {
        if (!NativeMethod.ortproxy_init(
                System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(Encoding.UTF8.GetBytes(path), 0)))
        {
            throw new Exception("OnnxRuntimeImpl.ApiProxy initialization failed!");
        }
    }
}