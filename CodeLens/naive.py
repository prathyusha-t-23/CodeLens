def naive_search(text, pattern):
    matches = []

    n = len(text)
    m = len(pattern)

    for i in range(n - m + 1):
        if text[i:i+m] == pattern:
            matches.append(i)

    return matches