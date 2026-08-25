# log_search.py
import sys
import os
import re
import argparse
from typing import List, Optional

def highlight(text: str, pattern: str, regex: bool) -> str:
    """Highlight matching text in the line."""
    if regex:
        return re.sub(pattern, f"\033[33m\\g<0>\033[0m", text)
    else:
        return text.replace(pattern, f"\033[33m{pattern}\033[0m")

def search_files(files: List[str], pattern: str, regex: bool, ignore_case: bool,
                 context: int, line_numbers: bool, count_only: bool, output: Optional[str]) -> int:
    """Search files and display results."""
    flags = re.IGNORECASE if ignore_case else 0
    if regex:
        try:
            compiled = re.compile(pattern, flags)
        except re.error as e:
            print(f"Regex error: {e}", file=sys.stderr)
            return 1
    else:
        compiled = None

    total_matches = 0
    results = []

    for filepath in files:
        if not os.path.exists(filepath):
            print(f"File not found: {filepath}", file=sys.stderr)
            continue
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()

        if count_only:
            match_count = 0
            for line in lines:
                if regex:
                    if compiled.search(line):
                        match_count += 1
                else:
                    if ignore_case:
                        if pattern.lower() in line.lower():
                            match_count += 1
                    else:
                        if pattern in line:
                            match_count += 1
            total_matches += match_count
            results.append(f"{filepath}: {match_count}")
            continue

        # Find matches with indices
        matched_lines = []
        for i, line in enumerate(lines):
            if regex:
                if compiled.search(line):
                    matched_lines.append((i, line))
            else:
                if ignore_case:
                    if pattern.lower() in line.lower():
                        matched_lines.append((i, line))
                else:
                    if pattern in line:
                        matched_lines.append((i, line))

        if context > 0:
            shown_indices = set()
            for idx, _ in matched_lines:
                for offset in range(-context, context + 1):
                    n = idx + offset
                    if 0 <= n < len(lines):
                        shown_indices.add(n)
            for idx in sorted(shown_indices):
                prefix = ">" if any(m[0] == idx for m in matched_lines) else " "
                line = lines[idx].rstrip('\n')
                if prefix == ">":
                    # Highlight the match
                    if regex:
                        line = re.sub(pattern, f"\033[33m\\g<0>\033[0m", line)
                    else:
                        line = line.replace(pattern, f"\033[33m{pattern}\033[0m")
                if line_numbers:
                    line_num = f"{idx+1:4d}"
                    results.append(f"{filepath}:{line_num}: {line}")
                else:
                    results.append(f"{filepath}: {line}")
            total_matches += len(matched_lines)
        else:
            for idx, line in matched_lines:
                # Highlight
                if regex:
                    line = re.sub(pattern, f"\033[33m\\g<0>\033[0m", line)
                else:
                    line = line.replace(pattern, f"\033[33m{pattern}\033[0m")
                if line_numbers:
                    line_num = f"{idx+1:4d}"
                    results.append(f"{filepath}:{line_num}: {line.rstrip()}")
                else:
                    results.append(f"{filepath}: {line.rstrip()}")
            total_matches += len(matched_lines)

    if output:
        with open(output, 'w') as f:
            if count_only:
                f.write("\n".join(results))
            else:
                f.write("\n".join(results))
                if total_matches:
                    f.write(f"\n\nTotal matches: {total_matches}\n")
        print(f"Results written to {output}")
    else:
        if count_only:
            for r in results:
                print(r)
            print(f"Total matches: {total_matches}")
        else:
            for r in results:
                print(r)
            if total_matches:
                print(f"\nTotal matches: {total_matches}")

    return 0

def main():
    parser = argparse.ArgumentParser(description="Log Search Tool")
    parser.add_argument('pattern', help='Search pattern')
    parser.add_argument('files', nargs='+', help='Log files to search')
    parser.add_argument('--regex', action='store_true', help='Treat pattern as regex')
    parser.add_argument('--ignore-case', action='store_true', help='Case-insensitive search')
    parser.add_argument('--context', type=int, default=0, help='Show N lines of context')
    parser.add_argument('--line-numbers', action='store_true', help='Show line numbers')
    parser.add_argument('--count', action='store_true', help='Show match count only')
    parser.add_argument('--output', help='Write results to a file')
    args = parser.parse_args()

    sys.exit(search_files(args.files, args.pattern, args.regex, args.ignore_case,
                          args.context, args.line_numbers, args.count, args.output))

if __name__ == "__main__":
    main()
