// See https://aka.ms/new-console-template for more information

namespace TestSetSeed;

public static class Program
{
    public static int Main(string[] args)
    {
        var seed = OrtSeedImpl.Library.GetRandomSeed();
        Console.WriteLine($"Original seed: {seed}");

        OrtSeedImpl.Library.SetRandomSeed(seed = new Random().NextInt64(0, Int64.MaxValue));
        Console.WriteLine($"Set a seed: {seed}");

        seed = OrtSeedImpl.Library.GetRandomSeed();
        Console.WriteLine($"Current seed: {seed}");

        Console.WriteLine($"Press enter to exit...");
        Console.ReadLine();
        return 0;
    }
}