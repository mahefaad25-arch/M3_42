def aa(text: str) -> tuple[int, str, int]:
    le = len(text)
    low = text.lower()
    count = 0
    for a in text.lower():
        if a in "aeiou":
            count = 1 + count
    return (le, low, count)

def cryptsort(text1: list[str]) -> list[str]:
    return sorted(text1, key=aa)

print(cryptsort(["sada","sdasd", "a"]))