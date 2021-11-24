import sys

raw = ""

with open(sys.argv[1], 'r') as f:
    raw = f.read()

raw_lines = raw.split("\n")
for l in raw_lines:
    print('"'+ l + '\\n"')
