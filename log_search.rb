# log_search.rb
#!/usr/bin/env ruby
require 'optparse'
require 'stringio'

def highlight(line, pattern, regex)
  if regex
    line.gsub(/#{pattern}/, "\e[33m\\0\e[0m")
  else
    line.gsub(pattern, "\e[33m#{pattern}\e[0m")
  end
end

def search_files(files, pattern, regex, ignore_case, context, line_numbers, count_only, output)
  total_matches = 0
  results = []

  files.each do |filepath|
    unless File.exist?(filepath)
      warn "File not found: #{filepath}"
      next
    end
    lines = File.readlines(filepath, chomp: true)

    if count_only
      match_count = 0
      lines.each do |line|
        if regex
          match_count += 1 if line.match?(Regexp.new(pattern, ignore_case))
        else
          if ignore_case
            match_count += 1 if line.downcase.include?(pattern.downcase)
          else
            match_count += 1 if line.include?(pattern)
          end
        end
      end
      total_matches += match_count
      results << "#{filepath}: #{match_count}"
      next
    end

    matched_indices = []
    lines.each_with_index do |line, idx|
      if regex
        matched_indices << idx if line.match?(Regexp.new(pattern, ignore_case))
      else
        if ignore_case
          matched_indices << idx if line.downcase.include?(pattern.downcase)
        else
          matched_indices << idx if line.include?(pattern)
        end
      end
    end

    if context > 0
      shown = []
      matched_indices.each do |idx|
        (idx - context..idx + context).each do |n|
          shown << n if n >= 0 && n < lines.length
        end
      end
      shown.uniq.sort.each do |idx|
        prefix = matched_indices.include?(idx) ? '>' : ' '
        line = lines[idx]
        line = highlight(line, pattern, regex) if prefix == '>'
        line_num = line_numbers ? "#{idx+1}".rjust(4) : ''
        results << "#{filepath}:#{line_num}: #{line}"
      end
      total_matches += matched_indices.length
    else
      matched_indices.each do |idx|
        line = lines[idx]
        line = highlight(line, pattern, regex)
        line_num = line_numbers ? "#{idx+1}".rjust(4) : ''
        results << "#{filepath}:#{line_num}: #{line}"
      end
      total_matches += matched_indices.length
    end
  end

  if output
    File.open(output, 'w') do |f|
      results.each { |r| f.puts r }
      f.puts "\nTotal matches: #{total_matches}" unless count_only
    end
    puts "Results written to #{output}"
  else
    puts results
    puts "\nTotal matches: #{total_matches}" unless count_only
  end
end

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: log_search.rb <pattern> <file1> [file2...] [options]"
  opts.on("--regex", "Treat pattern as regex") { options[:regex] = true }
  opts.on("--ignore-case", "Case-insensitive search") { options[:ignore_case] = true }
  opts.on("--context N", Integer, "Show N lines of context") { |v| options[:context] = v }
  opts.on("--line-numbers", "Show line numbers") { options[:line_numbers] = true }
  opts.on("--count", "Show match count only") { options[:count] = true }
  opts.on("--output FILE", "Write results to a file") { |v| options[:output] = v }
end.parse!

files = ARGV
if files.empty? || ARGV.empty?
  puts "Usage: log_search.rb <pattern> <file1> [file2...] [options]"
  exit 1
end

pattern = files.shift
search_files(files, pattern, options[:regex], options[:ignore_case], options[:context] || 0,
             options[:line_numbers], options[:count], options[:output])
