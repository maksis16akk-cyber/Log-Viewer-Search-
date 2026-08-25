// LogSearch.java
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

public class LogSearch {
    private static final String ANSI_RESET = "\033[0m";
    private static final String ANSI_YELLOW = "\033[33m";

    public static void main(String[] args) throws IOException {
        if (args.length < 2) {
            System.out.println("Usage: LogSearch <pattern> <file1> [file2...] [options]");
            System.out.println("Options: --regex, --ignore-case, --context N, --line-numbers, --count, --output FILE");
            return;
        }
        List<String> fileList = new ArrayList<>();
        String pattern = args[0];
        boolean regex = false, ignoreCase = false, lineNumbers = false, countOnly = false;
        int context = 0;
        String outputFile = null;
        for (int i = 1; i < args.length; i++) {
            if (args[i].equals("--regex")) regex = true;
            else if (args[i].equals("--ignore-case")) ignoreCase = true;
            else if (args[i].equals("--line-numbers")) lineNumbers = true;
            else if (args[i].equals("--count")) countOnly = true;
            else if (args[i].equals("--context") && i+1 < args.length) context = Integer.parseInt(args[++i]);
            else if (args[i].equals("--output") && i+1 < args.length) outputFile = args[++i];
            else fileList.add(args[i]);
        }
        if (fileList.isEmpty()) {
            System.err.println("No files specified.");
            System.exit(1);
        }
        searchFiles(fileList, pattern, regex, ignoreCase, context, lineNumbers, countOnly, outputFile);
    }

    private static void searchFiles(List<String> files, String pattern, boolean regex, boolean ignoreCase,
                                    int context, boolean lineNumbers, boolean countOnly, String outputFile) throws IOException {
        int totalMatches = 0;
        List<String> results = new ArrayList<>();
        Pattern compiled = null;
        if (regex) {
            int flags = ignoreCase ? Pattern.CASE_INSENSITIVE : 0;
            compiled = Pattern.compile(pattern, flags);
        }

        for (String filepath : files) {
            Path path = Paths.get(filepath);
            if (!Files.exists(path)) {
                System.err.println("File not found: " + filepath);
                continue;
            }
            List<String> lines = Files.readAllLines(path);

            if (countOnly) {
                int matchCount = 0;
                for (String line : lines) {
                    if (regex) {
                        if (compiled.matcher(line).find()) matchCount++;
                    } else {
                        if (ignoreCase) {
                            if (line.toLowerCase().contains(pattern.toLowerCase())) matchCount++;
                        } else {
                            if (line.contains(pattern)) matchCount++;
                        }
                    }
                }
                totalMatches += matchCount;
                results.add(filepath + ": " + matchCount);
                continue;
            }

            List<Integer> matchedIndices = new ArrayList<>();
            for (int i = 0; i < lines.size(); i++) {
                String line = lines.get(i);
                if (regex) {
                    if (compiled.matcher(line).find()) matchedIndices.add(i);
                } else {
                    if (ignoreCase) {
                        if (line.toLowerCase().contains(pattern.toLowerCase())) matchedIndices.add(i);
                    } else {
                        if (line.contains(pattern)) matchedIndices.add(i);
                    }
                }
            }

            if (context > 0) {
                Set<Integer> shown = new HashSet<>();
                for (int idx : matchedIndices) {
                    for (int offset = -context; offset <= context; offset++) {
                        int n = idx + offset;
                        if (n >= 0 && n < lines.size()) shown.add(n);
                    }
                }
                List<Integer> sorted = new ArrayList<>(shown);
                Collections.sort(sorted);
                for (int idx : sorted) {
                    String prefix = matchedIndices.contains(idx) ? ">" : " ";
                    String line = lines.get(idx);
                    if (prefix.equals(">")) {
                        line = highlight(line, pattern, regex);
                    }
                    String lineNum = lineNumbers ? String.format("%4d", idx+1) : "";
                    results.add(filepath + ":" + lineNum + ": " + line);
                }
                totalMatches += matchedIndices.size();
            } else {
                for (int idx : matchedIndices) {
                    String line = lines.get(idx);
                    line = highlight(line, pattern, regex);
                    String lineNum = lineNumbers ? String.format("%4d", idx+1) : "";
                    results.add(filepath + ":" + lineNum + ": " + line);
                }
                totalMatches += matchedIndices.size();
            }
        }

        if (outputFile != null) {
            Files.write(Paths.get(outputFile), results);
            if (!countOnly) {
                Files.write(Paths.get(outputFile), ("\nTotal matches: " + totalMatches).getBytes(), StandardOpenOption.APPEND);
            }
            System.out.println("Results written to " + outputFile);
        } else {
            results.forEach(System.out::println);
            if (!countOnly) System.out.println("\nTotal matches: " + totalMatches);
        }
    }

    private static String highlight(String line, String pattern, boolean regex) {
        if (regex) {
            return line.replaceAll(pattern, ANSI_YELLOW + "$0" + ANSI_RESET);
        }
        return line.replace(pattern, ANSI_YELLOW + pattern + ANSI_RESET);
    }
}
