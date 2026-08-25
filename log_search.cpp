// log_search.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <cctype>
#include <getopt.h>

using namespace std;

string highlight(const string& line, const string& pattern, bool regex) {
    if (regex) {
        regex re(pattern);
        return regex_replace(line, re, "\033[33m$&\033[0m");
    }
    string result = line;
    size_t pos = 0;
    while ((pos = result.find(pattern, pos)) != string::npos) {
        result.replace(pos, pattern.length(), "\033[33m" + pattern + "\033[0m");
        pos += pattern.length() + 11; // length of color codes
    }
    return result;
}

void searchFiles(const vector<string>& files, const string& pattern, bool regex, bool ignoreCase,
                 int context, bool lineNumbers, bool countOnly, const string& outputFile) {
    int totalMatches = 0;
    vector<string> results;
    regex compiled;
    bool compiledOk = false;
    if (regex) {
        try {
            compiled = regex(pattern, ignoreCase ? regex::icase : regex::ECMAScript);
            compiledOk = true;
        } catch (const regex_error& e) {
            cerr << "Regex error: " << e.what() << endl;
            return;
        }
    }

    for (const auto& filepath : files) {
        ifstream f(filepath);
        if (!f.is_open()) {
            cerr << "File not found: " << filepath << endl;
            continue;
        }
        vector<string> lines;
        string line;
        while (getline(f, line)) {
            lines.push_back(line);
        }
        f.close();

        if (countOnly) {
            int matchCount = 0;
            for (const auto& l : lines) {
                if (regex) {
                    if (regex_search(l, compiled)) matchCount++;
                } else {
                    if (ignoreCase) {
                        string lc = l, pc = pattern;
                        transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
                        transform(pc.begin(), pc.end(), pc.begin(), ::tolower);
                        if (lc.find(pc) != string::npos) matchCount++;
                    } else {
                        if (l.find(pattern) != string::npos) matchCount++;
                    }
                }
            }
            totalMatches += matchCount;
            results.push_back(filepath + ": " + to_string(matchCount));
            continue;
        }

        vector<int> matchedIndices;
        for (size_t i = 0; i < lines.size(); i++) {
            const auto& l = lines[i];
            if (regex) {
                if (regex_search(l, compiled)) matchedIndices.push_back(i);
            } else {
                if (ignoreCase) {
                    string lc = l, pc = pattern;
                    transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
                    transform(pc.begin(), pc.end(), pc.begin(), ::tolower);
                    if (lc.find(pc) != string::npos) matchedIndices.push_back(i);
                } else {
                    if (l.find(pattern) != string::npos) matchedIndices.push_back(i);
                }
            }
        }

        if (context > 0) {
            vector<int> shown;
            for (int idx : matchedIndices) {
                for (int offset = -context; offset <= context; offset++) {
                    int n = idx + offset;
                    if (n >= 0 && n < (int)lines.size()) shown.push_back(n);
                }
            }
            sort(shown.begin(), shown.end());
            shown.erase(unique(shown.begin(), shown.end()), shown.end());
            for (int idx : shown) {
                string prefix = (find(matchedIndices.begin(), matchedIndices.end(), idx) != matchedIndices.end()) ? ">" : " ";
                string line = lines[idx];
                if (prefix == ">") line = highlight(line, pattern, regex);
                string lineNum = lineNumbers ? to_string(idx+1) : "";
                if (!lineNum.empty()) lineNum = string(4 - min(4, (int)lineNum.length()), ' ') + lineNum;
                results.push_back(filepath + ":" + lineNum + ": " + line);
            }
            totalMatches += matchedIndices.size();
        } else {
            for (int idx : matchedIndices) {
                string line = lines[idx];
                line = highlight(line, pattern, regex);
                string lineNum = lineNumbers ? to_string(idx+1) : "";
                if (!lineNum.empty()) lineNum = string(4 - min(4, (int)lineNum.length()), ' ') + lineNum;
                results.push_back(filepath + ":" + lineNum + ": " + line);
            }
            totalMatches += matchedIndices.size();
        }
    }

    if (!outputFile.empty()) {
        ofstream out(outputFile);
        for (const auto& r : results) out << r << "\n";
        if (!countOnly) out << "\nTotal matches: " << totalMatches << "\n";
        out.close();
        cout << "Results written to " << outputFile << "\n";
    } else {
        for (const auto& r : results) cout << r << "\n";
        if (!countOnly) cout << "\nTotal matches: " << totalMatches << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: log_search <pattern> <file1> [file2...] [options]\n";
        cerr << "Options: --regex, --ignore-case, --context N, --line-numbers, --count, --output FILE\n";
        return 1;
    }

    string pattern = argv[1];
    vector<string> files;
    bool regex = false, ignoreCase = false, lineNumbers = false, countOnly = false;
    int context = 0;
    string outputFile;

    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--regex") regex = true;
        else if (arg == "--ignore-case") ignoreCase = true;
        else if (arg == "--line-numbers") lineNumbers = true;
        else if (arg == "--count") countOnly = true;
        else if (arg == "--context" && i+1 < argc) context = stoi(argv[++i]);
        else if (arg == "--output" && i+1 < argc) outputFile = argv[++i];
        else files.push_back(arg);
    }

    if (files.empty()) {
        cerr << "No files specified.\n";
        return 1;
    }

    searchFiles(files, pattern, regex, ignoreCase, context, lineNumbers, countOnly, outputFile);
    return 0;
}
