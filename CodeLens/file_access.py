def sequential_access(filepath):
    print("\nSequential Access:\n")

    with open(filepath, "r") as f:
        for line_no, line in enumerate(f, start=1):
            print(f"{line_no}: {line.strip()}")


def direct_access(filepath, target_line):
    print("\nDirect Access:\n")

    offsets = {}

    with open(filepath, "r") as f:

        while True:
            pos = f.tell()
            line = f.readline()

            if not line:
                break

            offsets[len(offsets) + 1] = pos

        if target_line in offsets:
            f.seek(offsets[target_line])
            print(f"Line {target_line}: {f.readline().strip()}")