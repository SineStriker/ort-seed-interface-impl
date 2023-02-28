// See https://aka.ms/new-console-template for more information

using System.Reflection;
using Microsoft.ML.OnnxRuntime;
using Microsoft.ML.OnnxRuntime.Tensors;

namespace TestSetSeed;

public static class Program
{
    public static int Main(string[] args)
    {
        var randGen = new Random();
        var getRandom = () => randGen.NextInt64(0, Int64.MaxValue);

        var sessionId = 1;

        // Set global id
        Console.WriteLine($"Set current session id as {sessionId}");
        OrtSeedImpl.Library.SetCurrentSessionId(sessionId);
                
        // Assertion
        if (OrtSeedImpl.Library.GetCurrentSessionId() != sessionId)
        {
            throw new Exception("Assertion failed setting or getting current session id from native library!");
        }

        // Load model
        Console.WriteLine("Load model");
        var session = new InferenceSession(Path.Join(
            Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "models", "random.onnx"));

        // Set seed
        var seed = getRandom();
        Console.WriteLine($"Set session seed: id {sessionId}, value {seed}");
        OrtSeedImpl.Library.SetSessionSeed(sessionId, seed);
        
        // Assertion
        if (OrtSeedImpl.Library.GetSessionSeed(sessionId) != seed)
        {
            throw new Exception("Assertion failed setting or getting session seed from native library!");
        }

        // Set task id
        var taskId = 1;
        Console.WriteLine($"Set session taskId: id {sessionId}, value {taskId}");
        OrtSeedImpl.Library.SetSessionTaskId(sessionId, taskId);
        
        // Assertion
        if (OrtSeedImpl.Library.GetSessionTaskId(sessionId) != taskId)
        {
            throw new Exception("Assertion failed setting or getting session taskId from native library!");
        }

        // Run once
        Console.WriteLine("Run once");
        var str1 = InferRandomModule(session).GetArrayString();

        // Set task id
        taskId = 2;
        Console.WriteLine($"Set session taskId: id {sessionId}, value {taskId}");
        OrtSeedImpl.Library.SetSessionTaskId(sessionId, taskId);
        
        // Assertion
        if (OrtSeedImpl.Library.GetSessionTaskId(sessionId) != taskId)
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
        OrtSeedImpl.Library.RemoveSession(sessionId);

        Console.WriteLine(
            $"Try get session seed after removing session: {OrtSeedImpl.Library.GetSessionTaskId(sessionId, -1)}");
        
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