// log_search.go
package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"regexp"
	"strings"
)

func highlight(line, pattern string, regex bool) string {
	if regex {
		re := regexp.MustCompile(pattern)
		return re.ReplaceAllString(line, "\033[33m$0\033[0m")
	}
	return strings.ReplaceAll(line, pattern, "\033[33m"+pattern+"\033[0m")
}

func searchFiles(files []string, pattern string, regex, ignoreCase bool, context int, lineNumbers, countOnly bool, output string) error {
	var totalMatches int
	var results []string

	flags := 0
	if ignoreCase {
		flags |= regexp.I
	}
	var compiled *regexp.Regexp
	if regex {
		var err error
		compiled, err = regexp.Compile(pattern)
		if err != nil {
			return fmt.Errorf("regex error: %v", err)
		}
	}

	for _, filepath := range files {
		f, err := os.Open(filepath)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Warning: %v\n", err)
			continue
		}
		defer f.Close()
		scanner := bufio.NewScanner(f)
		var lines []string
		for scanner.Scan() {
			lines = append(lines, scanner.Text())
		}
		if err := scanner.Err(); err != nil {
			fmt.Fprintf(os.Stderr, "Error reading %s: %v\n", filepath, err)
		}

		if countOnly {
			matchCount := 0
			for _, line := range lines {
				if regex {
					if compiled.MatchString(line) {
						matchCount++
					}
				} else {
					if ignoreCase {
						if strings.Contains(strings.ToLower(line), strings.ToLower(pattern)) {
							matchCount++
						}
					} else {
						if strings.Contains(line, pattern) {
							matchCount++
						}
					}
				}
			}
			totalMatches += matchCount
			results = append(results, fmt.Sprintf("%s: %d", filepath, matchCount))
			continue
		}

		// Collect matches
		var matchedIndices []int
		for i, line := range lines {
			if regex {
				if compiled.MatchString(line) {
					matchedIndices = append(matchedIndices, i)
				}
			} else {
				if ignoreCase {
					if strings.Contains(strings.ToLower(line), strings.ToLower(pattern)) {
						matchedIndices = append(matchedIndices, i)
					}
				} else {
					if strings.Contains(line, pattern) {
						matchedIndices = append(matchedIndices, i)
					}
				}
			}
		}

		if context > 0 {
			shown := make(map[int]bool)
			for _, idx := range matchedIndices {
				for offset := -context; offset <= context; offset++ {
					n := idx + offset
					if n >= 0 && n < len(lines) {
						shown[n] = true
					}
				}
			}
			for idx := 0; idx < len(lines); idx++ {
				if shown[idx] {
					prefix := " "
					for _, m := range matchedIndices {
						if m == idx {
							prefix = ">"
							break
						}
					}
					line := lines[idx]
					if prefix == ">" {
						line = highlight(line, pattern, regex)
					}
					if lineNumbers {
						results = append(results, fmt.Sprintf("%s:%4d: %s", filepath, idx+1, line))
					} else {
						results = append(results, fmt.Sprintf("%s: %s", filepath, line))
					}
				}
			}
			totalMatches += len(matchedIndices)
		} else {
			for _, idx := range matchedIndices {
				line := lines[idx]
				line = highlight(line, pattern, regex)
				if lineNumbers {
					results = append(results, fmt.Sprintf("%s:%4d: %s", filepath, idx+1, line))
				} else {
					results = append(results, fmt.Sprintf("%s: %s", filepath, line))
				}
			}
			totalMatches += len(matchedIndices)
		}
	}

	if output != "" {
		f, err := os.Create(output)
		if err != nil {
			return err
		}
		defer f.Close()
		for _, r := range results {
			f.WriteString(r + "\n")
		}
		if !countOnly {
			f.WriteString(fmt.Sprintf("\nTotal matches: %d\n", totalMatches))
		}
		fmt.Printf("Results written to %s\n", output)
	} else {
		for _, r := range results {
			fmt.Println(r)
		}
		if !countOnly {
			fmt.Printf("\nTotal matches: %d\n", totalMatches)
		}
	}
	return nil
}

func main() {
	var (
		pattern     = flag.String("pattern", "", "Search pattern")
		regexFlag   = flag.Bool("regex", false, "Treat pattern as regex")
		ignoreCase  = flag.Bool("ignore-case", false, "Case-insensitive search")
		context     = flag.Int("context", 0, "Show N lines of context")
		lineNumbers = flag.Bool("line-numbers", false, "Show line numbers")
		countOnly   = flag.Bool("count", false, "Show match count only")
		output      = flag.String("output", "", "Write results to a file")
	)
	flag.Parse()

	files := flag.Args()
	if *pattern == "" || len(files) == 0 {
		fmt.Println("Usage: log_search <pattern> <file1> [file2...] [options]")
		os.Exit(1)
	}

	if err := searchFiles(files, *pattern, *regexFlag, *ignoreCase, *context, *lineNumbers, *countOnly, *output); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}
}
