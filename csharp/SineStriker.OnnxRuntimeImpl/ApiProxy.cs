using System.Text;

namespace SineStriker.OnnxRuntimeImpl;

public static class ApiProxy
{
    /// <summary>
    /// Initialize proxy library
    /// </summary>
    /// <param name="dev">Real OnnxRuntime library directory, evaluated relative to application directory if relative</param>
    public static void Init(string dev)
    {
        var devBytes = Encoding.UTF8.GetBytes(dev);
        if (!NativeMethod.ortproxy_init(
                System.Runtime.InteropServices.Marshal.UnsafeAddrOfPinnedArrayElement(devBytes, 0)))
        {
            throw new Exception("OnnxRuntimeImpl.ApiProxy initialization failed!");
        }
    }
}