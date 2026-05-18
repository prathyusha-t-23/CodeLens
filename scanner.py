import os
from horspool import horspool_search


def load_patterns(pattern_file):
    with open(pattern_file, "r") as f:
        return [line.strip() for line in f]


def scan_file(filepath, patterns):
    results = []

    with open(filepath, "r") as f:
        lines = f.readlines()

    for line_no, line in enumerate(lines, start=1):

        for pattern in patterns:

            matches = horspool_search(line, pattern)

            if matches:
                results.append({
                    "pattern": pattern,
                    "line_no": line_no,
                    "line": line.strip()
                })

    return results


def scan_directory(directory, patterns):

    all_results = {}

    for root, dirs, files in os.walk(directory):

        for file in files:

            if file.endswith((".py", ".java", ".c")):

                path = os.path.join(root, file)

                result = scan_file(path, patterns)

                if result:
                    all_results[path] = result

    return all_results