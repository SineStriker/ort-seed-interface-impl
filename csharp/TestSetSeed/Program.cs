// See https://aka.ms/new-console-template for more information

using System.ComponentModel;
using Microsoft.ML.OnnxRuntime;
using Microsoft.ML.OnnxRuntime.Tensors;
using SineStriker.OnnxRuntimeImpl;

namespace TestSetSeed;

public static class Program
{
    enum ExecutionProviderType
    {
        [Description("dml")] DirectML,
        [Description("cpu")] CPU,
        [Description("cuda")] CUDA,
    }

    private const ExecutionProviderType _epType = ExecutionProviderType.CPU;

    public static int Main(string[] args)
    {
        ApiProxy.Init($"eps/{Utils.GetEnumDesc(_epType)}", true);
        Console.WriteLine();

        // Add env for cuda
        if (_epType == ExecutionProviderType.CUDA)
        {
            Utils.AddPath(@"C:\Program Files\NVIDIA\CUDNN\v8.8.0.121\bin");
            Utils.AddPath(@"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.5\bin");
        }

        var randGen = new System.Random();
        var getRandom = () => randGen.NextInt64(0, Int64.MaxValue);

        var sessionId = getRandom();

        // Set global id
        Console.WriteLine("[Test SetCurrentSessionId]");
        Sessions.SetCurrentSessionId(sessionId);
        Utils.Assert(Sessions.GetCurrentSessionId() == sessionId);
        Console.WriteLine("Pass");

        // Load model
        Console.WriteLine("[Load model]");

        SessionOptions opt;
        switch (_epType)
        {
            case ExecutionProviderType.DirectML:
                opt = new SessionOptions();
                opt.AppendExecutionProvider_DML(0);
                break;
            case ExecutionProviderType.CUDA:
                opt = SessionOptions.MakeSessionOptionWithCudaProvider();
                break;
            default:
                opt = new();
                break;
        }

        var session = new InferenceSession(
            Path.Join(Utils.ExeDir, "models", "random.onnx"),
            opt
        );

        // Set seed
        var seed = getRandom();
        Console.WriteLine("[Test SetSessionSeed]");
        Sessions.SetSessionSeed(sessionId, seed);
        Utils.Assert(Sessions.GetSessionSeed(sessionId) == seed);
        Console.WriteLine("Pass");

        // Set task id
        var taskId = 1;
        Console.WriteLine("[Test SetTaskId]");
        Sessions.SetSessionTaskId(sessionId, taskId);
        Utils.Assert(Sessions.GetSessionTaskId(sessionId) == taskId);
        Console.WriteLine("Pass");

        // Run once
        Console.WriteLine("[Test Run]");
        var str1 = InferRandomModule(session).GetArrayString();
        Console.WriteLine("Pass");

        // Set task id
        taskId = 2;
        Console.WriteLine("[Test SetTaskId]");
        Sessions.SetSessionTaskId(sessionId, taskId);
        Utils.Assert(Sessions.GetSessionTaskId(sessionId) == taskId);
        Console.WriteLine("Pass");

        // Run twice
        Console.WriteLine("[Test Stability]");
        var str2 = InferRandomModule(session).GetArrayString();
        // Console.WriteLine(str1 == str2 ? "Output tensors match!" : "Output tensors mismatch!");
        Utils.Assert(str1 == str2);
        Console.WriteLine("Pass");

        // Run again without updating task id
        Console.WriteLine("[Test Instability]");
        var str3 = InferRandomModule(session).GetArrayString();
        // Console.WriteLine(str2 == str3 ? "Output tensors match!" : "Output tensors mismatch!");
        Utils.Assert(str2 != str3);
        Console.WriteLine("Pass");

        // Dispose session
        Console.WriteLine("[Test RemoveSession]");
        session.Dispose();
        Sessions.RemoveSession(sessionId);
        Utils.Assert(Sessions.GetSessionTaskId(sessionId, -1) == -1);
        Console.WriteLine("Pass");

        return 0;
    }

    public static Tensor<float> InferRandomModule(InferenceSession session)
    {
        var input = new DenseTensor<float>(new[] { 1, 32, 5 });
        input.Fill(0f);
        var x = NamedOnnxValue.CreateFromTensor("x", input);
        var y = session.Run(new[] { x }).ElementAt(0);
        var output = y.AsTensor<float>();
        return output;
    }
}