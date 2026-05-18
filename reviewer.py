import time

from scanner import load_patterns, scan_directory
from report_generator import generate_report
from file_access import sequential_access, direct_access

from naive import naive_search
from horspool import horspool_search, build_shift_table


patterns = load_patterns("pattern.txt")

results = scan_directory("test_files", patterns)

generate_report(results, "report.txt")

print("\nReport Generated -> report.txt")


# DAA PERFORMANCE TEST

large_text = "A" * 100000 + "TODO"

pattern = "TODO"

start = time.time()
naive_search(large_text, pattern)
end = time.time()

print(f"\nNaive Time: {end - start:.6f} sec")


start = time.time()
horspool_search(large_text, pattern)
end = time.time()

print(f"Horspool Time: {end - start:.6f} sec")


# SHIFT TABLE

print("\nShift Table:")
print(build_shift_table(pattern))


# OS DEMO

sample_file = "test_files/sample.py"

sequential_access(sample_file)

direct_access(sample_file, 3)