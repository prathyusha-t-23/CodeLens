def generate_report(results, output_file):

    with open(output_file, "w") as f:

        for file, issues in results.items():

            f.write(f"\nFILE: {file}\n")
            f.write("-" * 40 + "\n")

            for issue in issues:

                f.write(f"Pattern : {issue['pattern']}\n")
                f.write(f"Line No : {issue['line_no']}\n")
                f.write(f"Code    : {issue['line']}\n")
                f.write("\n")