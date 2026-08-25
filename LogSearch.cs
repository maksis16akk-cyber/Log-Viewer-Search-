// LogSearch.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

class LogSearch
{
    static void Main(string[] args)
    {
        if (args.Length < 2)
        {
            Console.WriteLine("Usage: LogSearch <pattern> <file1> [file2...] [options]");
            Console.WriteLine("Options: --regex, --ignore-case, --context N, --line-numbers, --count, --output FILE");
            return;
        }
        var files = new List<string>();
        string pattern = args[0];
        bool regex = false, ignoreCase = false, lineNumbers = false, countOnly = false;
        int context = 0;
        string outputFile = null;
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i] == "--regex") regex = true;
            else if (args[i] == "--ignore-case") ignoreCase = true;
            else if (args[i] == "--line-numbers") lineNumbers = true;
            else if (args[i] == "--count") countOnly = true;
            else if (args[i] == "--context" && i+1 < args.Length) context = int.Parse(args[++i]);
            else if (args[i] == "--output" && i+1 < args.Length) outputFile = args[++i];
            else files.Add(args[i]);
        }
        if (files.Count == 0)
        {
            Console.Error.WriteLine("No files specified.");
            Environment.Exit(1);
        }
        SearchFiles(files, pattern, regex, ignoreCase, context, lineNumbers, countOnly, outputFile);
    }

    static void SearchFiles(List<string> files, string pattern, bool regex, bool ignoreCase,
                            int context, bool lineNumbers, bool countOnly, string outputFile)
    {
        int totalMatches = 0;
        var results = new List<string>();
        Regex compiled = null;
        if (regex)
        {
            var options = ignoreCase ? RegexOptions.IgnoreCase : RegexOptions.None;
            compiled = new Regex(pattern, options);
        }

        foreach (var filepath in files)
        {
            if (!File.Exists(filepath))
            {
                Console.Error.WriteLine($"File not found: {filepath}");
                continue;
            }
            var lines = File.ReadAllLines(filepath);

            if (countOnly)
            {
                int matchCount = 0;
                foreach (var line in lines)
                {
                    if (regex)
                    {
                        if (compiled.IsMatch(line)) matchCount++;
                    }
                    else
                    {
                        if (ignoreCase)
                        {
                            if (line.IndexOf(pattern, StringComparison.OrdinalIgnoreCase) >= 0) matchCount++;
                        }
                        else
                        {
                            if (line.Contains(pattern)) matchCount++;
                        }
                    }
                }
                totalMatches += matchCount;
                results.Add($"{filepath}: {matchCount}");
                continue;
            }

            var matchedIndices = new List<int>();
            for (int i = 0; i < lines.Length; i++)
            {
                var line = lines[i];
                if (regex)
                {
                    if (compiled.IsMatch(line)) matchedIndices.Add(i);
                }
                else
                {
                    if (ignoreCase)
                    {
                        if (line.IndexOf(pattern, StringComparison.OrdinalIgnoreCase) >= 0) matchedIndices.Add(i);
                    }
                    else
                    {
                        if (line.Contains(pattern)) matchedIndices.Add(i);
                    }
                }
            }

            if (context > 0)
            {
                var shown = new HashSet<int>();
                foreach (var idx in matchedIndices)
                {
                    for (int offset = -context; offset <= context; offset++)
                    {
                        int n = idx + offset;
                        if (n >= 0 && n < lines.Length) shown.Add(n);
                    }
                }
                foreach (var idx in shown.OrderBy(x => x))
                {
                    string prefix = matchedIndices.Contains(idx) ? ">" : " ";
                    string line = lines[idx];
                    if (prefix == ">") line = Highlight(line, pattern, regex);
                    string lineNum = lineNumbers ? $"{idx+1,4}" : "";
                    results.Add($"{filepath}:{lineNum}: {line}");
                }
                totalMatches += matchedIndices.Count;
            }
            else
            {
                foreach (var idx in matchedIndices)
                {
                    string line = lines[idx];
                    line = Highlight(line, pattern, regex);
                    string lineNum = lineNumbers ? $"{idx+1,4}" : "";
                    results.Add($"{filepath}:{lineNum}: {line}");
                }
                totalMatches += matchedIndices.Count;
            }
        }

        if (outputFile != null)
        {
            File.WriteAllLines(outputFile, results);
            if (!countOnly) File.AppendAllText(outputFile, $"\nTotal matches: {totalMatches}\n");
            Console.WriteLine($"Results written to {outputFile}");
        }
        else
        {
            results.ForEach(Console.WriteLine);
            if (!countOnly) Console.WriteLine($"\nTotal matches: {totalMatches}");
        }
    }

    static string Highlight(string line, string pattern, bool regex)
    {
        if (regex)
        {
            return Regex.Replace(line, pattern, "\x1b[33m$0\x1b[0m");
        }
        return line.Replace(pattern, "\x1b[33m" + pattern + "\x1b[0m");
    }
}
