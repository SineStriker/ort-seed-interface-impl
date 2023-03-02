using System.ComponentModel;
using System.Reflection;

namespace TestSetSeed;

public static class Utils
{
    public static readonly string ExeDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!;

    public static string GetEnumDesc<T>(T tField)
    {
        if (tField == null) throw new ArgumentNullException(nameof(tField));
        var description = string.Empty; //结果
        var inputType = tField.GetType(); //输入的类型
        var descType = typeof(DescriptionAttribute); //目标查找的描述类型
        var fieldStr = tField.ToString()!; //输入的字段字符串
        var field = inputType.GetField(fieldStr)!; //目标字段

        var isDefined = field.IsDefined(descType, false); //判断描述是否在字段的特性
        if (isDefined)
        {
            var enumAttributes = (DescriptionAttribute[])field //得到特性信息
                .GetCustomAttributes(descType, false);
            description = enumAttributes.FirstOrDefault()?.Description ?? string.Empty;
        }

        return description;
    }

    public static void AddPath(string path)
    {
        Environment.SetEnvironmentVariable("PATH",
            Environment.GetEnvironmentVariable("PATH") + ";" + path.Replace("/", "\\"));
    }

    public static void Assert(bool b)
    {
        if (!b)
        {
            throw new Exception("Assertion failed!");
        }
    }
}