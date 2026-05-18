def build_shift_table(pattern):
    m = len(pattern)
    table = {}

    for i in range(m - 1):
        table[pattern[i]] = m - 1 - i

    return table


def horspool_search(text, pattern):
    matches = []

    m = len(pattern)
    n = len(text)

    table = build_shift_table(pattern)

    i = m - 1

    while i < n:
        k = 0

        while k < m and pattern[m - 1 - k] == text[i - k]:
            k += 1

        if k == m:
            matches.append(i - m + 1)

        shift_char = text[i]

        i += table.get(shift_char, m)

    return matches