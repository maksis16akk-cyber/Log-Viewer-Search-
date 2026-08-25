📜 Log Viewer (Search) — Multi‑Language Log Searcher
8 languages, one powerful log search tool – find patterns in log files using regex, keywords, case‑insensitive search, context lines, and more – right from your terminal.

✨ Features
🔍 Keyword search – find exact matches in log files

🧩 Regex support – use regular expressions for advanced pattern matching

📏 Case‑insensitive mode – ignore case when searching

📋 Line numbers – show line numbers for each match

📐 Context lines – display lines before and after each match

📊 Match count – count total matches across files

📁 Multiple files – search across several log files at once

🎨 Highlighting – highlight matching text (where supported)

💾 Output to file – save results to a text file

🧰 Supported Languages & Files
Language	File	Dependencies
Python	log_search.py	none (stdlib)
Go	log_search.go	none (stdlib)
JavaScript (Node)	log_search.js	commander (optional)
Ruby	log_search.rb	optparse (stdlib)
PHP	log_search.php	none (extensions)
Java	LogSearch.java	Java 8+
C#	LogSearch.cs	.NET Core 3.1+
C++	log_search.cpp	nlohmann/json (optional)
🚀 Quick Start
All implementations follow the same CLI pattern:

bash
# Basic search
<command> search "ERROR" app.log

# Case‑insensitive search
<command> search "warning" --ignore-case system.log

# Regex search
<command> search "ERROR.*timeout" --regex app.log

# Show 2 lines of context
<command> search "Failed" --context 2 app.log

# Show line numbers
<command> search "Exception" --line-numbers app.log

# Count matches only
<command> search "error" --count app.log

# Search multiple files
<command> search "INFO" app1.log app2.log app3.log

# Save results to a file
<command> search "DEBUG" app.log --output results.txt
Arguments:

<pattern> – search pattern (keyword or regex)

<files...> – one or more log files to search

--regex – treat pattern as regular expression

--ignore-case – case‑insensitive search

--context N – show N lines before and after each match

--line-numbers – show line numbers

--count – show total match count only

--output <file> – write results to a file

--help – show usage

📸 Example Output
text
🔍 Log Search
Pattern: ERROR
Files: app.log
Options: line‑numbers, context=2

app.log:12: [2026-08-25 10:15:32] ERROR Database connection failed
app.log-13: [2026-08-25 10:15:33] INFO Retrying in 5 seconds
app.log-14: [2026-08-25 10:15:34] INFO Reconnection attempt 1

app.log:45: [2026-08-25 10:20:10] ERROR Timeout occurred
app.log-46: [2026-08-25 10:20:11] INFO Connection closed

Total matches: 2
📁 Repository Structure
text
.
├── README.md
├── python/
│   └── log_search.py
├── go/
│   └── log_search.go
├── javascript/
│   └── log_search.js
├── ruby/
│   └── log_search.rb
├── php/
│   └── log_search.php
├── java/
│   └── LogSearch.java
├── csharp/
│   └── LogSearch.cs
└── cpp/
    └── log_search.cpp
