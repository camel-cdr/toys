#!/usr/bin/env python3

import sys

def main():
    for path in sys.argv[1:]:
        with open(path, 'rt') as f:
            comments = extract_comments(f)
            if comments:
                print(f"// {path}")
                for lines in comments:
                    generate(lines)


def extract_comments(file):
    comments = []

    inside = False
    for line in file:
        if inside:
            idx = line.find('*/')
            if idx >= 0:
                line = line.replace('*/', '')
                inside = False

            comments[-1].append(line)
        else:
            idx = line.find('/**')
            if idx < 0:
                continue

            if idx == line.find('/**/'):
                continue

            tmp = line.split()
            try:
                inside = (tmp[1] == "pshufb")
            except IndexError:
                pass

            comments.append([])

    assert inside == False, "Comment not closed"

    return comments


def unindent(lines):
    n = max(map(len, lines))
    lines = [line.rstrip() for line in lines]
    for line in lines:
        if not line:
            continue
        k = len(line) - len(line.lstrip())
        n = min(k, n)

    return [line[n:].rstrip() for line in lines]


def print_constants(name, bytes, rep=1):
    assert len(bytes) * rep == 64
    allbytes = ['%02x' % byte for byte in rep * bytes]

    print(f"const __m512i {name} = _mm512_setr_epi64(")
    for k, i in enumerate(range(0, 64, 8)):
        qword = allbytes[i:i + 8]
        image = '0x' + ''.join(reversed(qword))
        if k < 7:
            print(f"    {image},")
        else:
            print(f"    {image}")
    print(");")


def generate(lines):
    program = ''.join(['if True:\n'] + lines)
    objects = {}
    try:
        exec(program, objects)
    except:
        raise ValueError(f"Cannot evaluate program")

    for variable, values in objects.items():
        if isinstance(values, type([])) and len(values) == 64:
            print_constants(variable, values)
            return

    raise ValueError(f"The program does not define any 64-value array")


if __name__ == '__main__':
    main()
