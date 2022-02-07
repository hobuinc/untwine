import os
from collections import defaultdict

def parent(name):
    arr = name.split('-')
    p = ''
    parts = name.split('-')
    if not validfile(name) or parts[0] == '0':
        return ""
    p += str(int(parts[0]) - 1)
    p += '-'
    p += str(int(parts[1]) // 2)
    p += '-'
    p += str(int(parts[2]) // 2)
    p += '-'
    p += str(int(parts[3]) // 2)
    return p

def children(name):
    l = []
    parts = name.split('-')
    level = int(parts[0]) + 1;
    for x in range(2):
        for y in range(2):
            for z in range(2):
                c = str(level) + '-'
                c += str(int(parts[1]) * 2 + x) + '-'
                c += str(int(parts[2]) * 2 + y) + '-'
                c += str(int(parts[3]) * 2 + z)
                l.append(c)
    return l

def validfile(name):
    parts = name.split('-')
    return parts[0].isnumeric()
    
arr = os.listdir('.')
totals = defaultdict(lambda: int(0))
for f in arr:
    if not validfile(f):
        continue
    numpts = os.path.getsize(f) / 52
    p = os.path.splitext(f)[0];
    totals[p] += numpts
    while p := parent(p):
        totals[p] += numpts

def printNode(p, level):
    for _ in range(level):
        print('\t', end='')
    print(p, " = ", totals[p])
    childs = children(p)
    for c in childs:
        if c in totals:
            printNode(c, level + 1)

p = '0-0-0-0'
level = 0;
printNode(p, level)
