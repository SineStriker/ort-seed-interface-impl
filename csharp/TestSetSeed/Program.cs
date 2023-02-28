﻿// See https://aka.ms/new-console-template for more information

using System.Reflection;
using Microsoft.ML.OnnxRuntime;
using Microsoft.ML.OnnxRuntime.Tensors;

namespace TestSetSeed;

public static class Program
{
    public static int Main(string[] args)
    {
        var seed = OrtSeedImpl.Library.GetRandomSeed();
        Console.WriteLine($"Original seed: {seed}");

        seed = new Random().NextInt64(0, Int64.MaxValue);
        
        OrtSeedImpl.Library.SetRandomSeed(seed);
        Console.WriteLine($"Set a seed: {seed}");

        seed = OrtSeedImpl.Library.GetRandomSeed();
        Console.WriteLine($"Current seed: {seed}");

        var str1 = InferRandomModule().GetArrayString();
        
        OrtSeedImpl.Library.SetRandomSeed(seed);
        Console.WriteLine($"Set a seed: {seed}");

        seed = OrtSeedImpl.Library.GetRandomSeed();
        Console.WriteLine($"Current seed: {seed}");

        var str2 = InferRandomModule().GetArrayString();

        Console.WriteLine(str1 == str2 ? "Output tensors match!" : "Output tensors mismatch!");

        Console.WriteLine($"Press enter to exit...");
        Console.ReadLine();
        return 0;
    }

    public static Tensor<float> InferRandomModule()
    {
        var modelPath = Path.Join(
            Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "models", "random.onnx");
        var session = new InferenceSession(modelPath);
        var input = new DenseTensor<float>(new[] { 1, 32, 5 });
        input.Fill(0f);
        var x = NamedOnnxValue.CreateFromTensor("x", input);
        var y = session.Run(new[] { x }).ElementAt(0);
        var output = y.AsTensor<float>();
        return output;
    }
}