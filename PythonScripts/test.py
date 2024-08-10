def find_longest_repeated_pattern(boolean_list):
    patterns = {}
    n = len(boolean_list)

    for i in range(n):
        for j in range(i + 1, n + 1):
            pattern = tuple(boolean_list[i:j])
            if pattern in patterns:
                patterns[pattern] += 1
            else:
                patterns[pattern] = 1

    # Filter out patterns that only occur more than once
    repeated_patterns = {pattern: count for pattern, count in patterns.items() if count > 1}

    # Find the longest repeated pattern
    if repeated_patterns:
        longest_pattern = max(repeated_patterns, key=len)
        return longest_pattern, repeated_patterns[longest_pattern]
    else:
        return None, 0

# Example usage
boolean_list = [True, True, False, True, False, True, False, True, True, False, True, False, True, False]
longest_pattern, count = find_longest_repeated_pattern(boolean_list)

if longest_pattern:
    print(longest_pattern[-1])
