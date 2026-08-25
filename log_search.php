# log_search.php
#!/usr/bin/env php
<?php

function highlight($line, $pattern, $regex) {
    if ($regex) {
        return preg_replace('/' . preg_quote($pattern) . '/', "\033[33m\\0\033[0m", $line);
    }
    return str_replace($pattern, "\033[33m{$pattern}\033[0m", $line);
}

function searchFiles($files, $pattern, $regex, $ignoreCase, $context, $lineNumbers, $countOnly, $output) {
    $totalMatches = 0;
    $results = [];

    foreach ($files as $filepath) {
        if (!file_exists($filepath)) {
            fwrite(STDERR, "File not found: $filepath\n");
            continue;
        }
        $lines = file($filepath, FILE_IGNORE_NEW_LINES);
        if ($lines === false) continue;

        if ($countOnly) {
            $matchCount = 0;
            foreach ($lines as $line) {
                if ($regex) {
                    if (preg_match('/' . preg_quote($pattern) . '/', $line)) $matchCount++;
                } else {
                    if ($ignoreCase) {
                        if (stripos($line, $pattern) !== false) $matchCount++;
                    } else {
                        if (strpos($line, $pattern) !== false) $matchCount++;
                    }
                }
            }
            $totalMatches += $matchCount;
            $results[] = "$filepath: $matchCount";
            continue;
        }

        $matchedIndices = [];
        foreach ($lines as $idx => $line) {
            if ($regex) {
                if (preg_match('/' . preg_quote($pattern) . '/', $line)) $matchedIndices[] = $idx;
            } else {
                if ($ignoreCase) {
                    if (stripos($line, $pattern) !== false) $matchedIndices[] = $idx;
                } else {
                    if (strpos($line, $pattern) !== false) $matchedIndices[] = $idx;
                }
            }
        }

        if ($context > 0) {
            $shown = [];
            foreach ($matchedIndices as $idx) {
                for ($offset = -$context; $offset <= $context; $offset++) {
                    $n = $idx + $offset;
                    if ($n >= 0 && $n < count($lines)) $shown[] = $n;
                }
            }
            $shown = array_unique($shown);
            sort($shown);
            foreach ($shown as $idx) {
                $prefix = in_array($idx, $matchedIndices) ? '>' : ' ';
                $line = $lines[$idx];
                if ($prefix === '>') $line = highlight($line, $pattern, $regex);
                $lineNum = $lineNumbers ? str_pad($idx+1, 4, ' ', STR_PAD_LEFT) : '';
                $results[] = "$filepath:$lineNum: $line";
            }
            $totalMatches += count($matchedIndices);
        } else {
            foreach ($matchedIndices as $idx) {
                $line = $lines[$idx];
                $line = highlight($line, $pattern, $regex);
                $lineNum = $lineNumbers ? str_pad($idx+1, 4, ' ', STR_PAD_LEFT) : '';
                $results[] = "$filepath:$lineNum: $line";
            }
            $totalMatches += count($matchedIndices);
        }
    }

    if ($output) {
        file_put_contents($output, implode("\n", $results) . "\n");
        if (!$countOnly) file_put_contents($output, "\nTotal matches: $totalMatches\n", FILE_APPEND);
        echo "Results written to $output\n";
    } else {
        echo implode("\n", $results) . "\n";
        if (!$countOnly) echo "\nTotal matches: $totalMatches\n";
    }
}

$opts = getopt("", ["regex", "ignore-case", "context:", "line-numbers", "count", "output:"]);
$args = array_slice($argv, 1);
foreach ($args as $i => $arg) {
    if (strpos($arg, '--') === 0) unset($args[$i]);
}
$args = array_values($args);
if (count($args) < 2) {
    echo "Usage: php log_search.php <pattern> <file1> [file2...] [options]\n";
    exit(1);
}
$pattern = array_shift($args);
$files = $args;
$regex = isset($opts['regex']);
$ignoreCase = isset($opts['ignore-case']);
$context = isset($opts['context']) ? (int)$opts['context'] : 0;
$lineNumbers = isset($opts['line-numbers']);
$countOnly = isset($opts['count']);
$output = $opts['output'] ?? null;

searchFiles($files, $pattern, $regex, $ignoreCase, $context, $lineNumbers, $countOnly, $output);
?>
