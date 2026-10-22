"""Copy named top-level C functions (and #defines) out of an OPL source file.

Usage: extract.py <source.c> <name> [<name> ...]
A name matches a function definition (`... name(` at column 0, body up to the
next `}` at column 0) or a `#define name`. Output goes to stdout in source order.
"""
import re
import sys


def main():
    path, names = sys.argv[1], set(sys.argv[2:])
    lines = open(path, encoding="latin-1").read().splitlines()
    out, found = [], set()
    i = 0
    while i < len(lines):
        line = lines[i]
        m = re.match(r"#define\s+(\w+)", line)
        if m and m.group(1) in names:
            out.append(line)
            found.add(m.group(1))
            i += 1
            continue
        m = re.match(r"^struct\s+(\w+)\s*$", line)
        if m and m.group(1) in names:
            j = i
            while not lines[j].startswith("}"):
                j += 1
            out.extend(lines[i : j + 1])
            out.append("")
            found.add(m.group(1))
            i = j + 1
            continue
        m = re.match(r"^[A-Za-z_].*?\b(\w+)\s*\(", line)
        if m and m.group(1) in names and not line.rstrip().endswith(";"):
            j = i
            while lines[j] != "}":
                j += 1
            out.extend(lines[i : j + 1])
            out.append("")
            found.add(m.group(1))
            i = j + 1
            continue
        i += 1
    missing = names - found
    if missing:
        sys.exit("not found in %s: %s" % (path, ", ".join(sorted(missing))))
    print("\n".join(out))


main()
