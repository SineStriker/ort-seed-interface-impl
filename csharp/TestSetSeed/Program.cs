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

    private const ExecutionProviderType _epType = ExecutionProviderType.DirectML;

    public static int Main(string[] args)
    {
        ApiProxy.Init($"eps/{Utils.GetEnumDesc(_epType)}");
        Console.WriteLine();

        // Add env for cuda
        if (_epType == ExecutionProviderType.CUDA)
        {
            Utils.AddPath(@"C:\Program Files\NVIDIA\CUDNN\v8.8.0.121\bin");
            Utils.AddPath(@"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.5\bin");
        }

        var randGen = new System.Random();
        var getRandom = () => randGen.NextInt64(0, Int64.MaxValue);

        var sessionId = 1;

        // Set global id
        Console.WriteLine($"Set current session id as {sessionId}");
        Stablize.SetCurrentSessionId(sessionId);

        // Assertion
        if (Stablize.GetCurrentSessionId() != sessionId)
        {
            throw new Exception("Assertion failed setting or getting current session id from native library!");
        }

        // Load model
        Console.WriteLine("Load model");

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
        Console.WriteLine($"Set session seed: id {sessionId}, value {seed}");
        Stablize.SetSessionSeed(sessionId, seed);

        // Assertion
        if (Stablize.GetSessionSeed(sessionId) != seed)
        {
            throw new Exception("Assertion failed setting or getting session seed from native library!");
        }

        // Set task id
        var taskId = 1;
        Console.WriteLine($"Set session taskId: id {sessionId}, value {taskId}");
        Stablize.SetSessionTaskId(sessionId, taskId);

        // Assertion
        if (Stablize.GetSessionTaskId(sessionId) != taskId)
        {
            throw new Exception("Assertion failed setting or getting session taskId from native library!");
        }

        // Run once
        Console.WriteLine("Run once");
        var str1 = InferRandomModule(session).GetArrayString();

        // Set task id
        taskId = 2;
        Console.WriteLine($"Set session taskId: id {sessionId}, value {taskId}");
        Stablize.SetSessionTaskId(sessionId, taskId);

        // Assertion
        if (Stablize.GetSessionTaskId(sessionId) != taskId)
        {
            throw new Exception("Assertion failed setting or getting session taskId from native library!");
        }

        // Run twice
        Console.WriteLine("Run twice");
        var str2 = InferRandomModule(session).GetArrayString();
        Console.WriteLine(str1 == str2 ? "Output tensors match!" : "Output tensors mismatch!");

        // Run again without updating task id
        Console.WriteLine("Run third");
        var str3 = InferRandomModule(session).GetArrayString();
        Console.WriteLine(str2 == str3 ? "Output tensors match!" : "Output tensors mismatch!");

        // Dispose session
        session.Dispose();
        Stablize.RemoveSession(sessionId);

        Console.WriteLine(
            $"Try get session seed after removing session: {Stablize.GetSessionTaskId(sessionId, -1)}");

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