#!/usr/bin/env python3
import subprocess, random, sys
sys.set_int_max_str_digits(1000000)

BIN = sys.argv[1] if len(sys.argv) > 1 else "./build/test_main"

random.seed(12345)

def rand_int(max_digits):
    nd = random.randint(1, max_digits)
    neg = random.choice([False, True])
    s = ''.join(random.choice('0123456789') for _ in range(nd))
    val = int(s)
    if neg and val != 0:
        val = -val
    return val

def gen(trials, max_digits):
    cases = []
    for _ in range(trials):
        a = rand_int(max_digits)
        b = rand_int(max_digits)
        if b == 0:
            b = 1
        cases.append((a, b))
    return cases

# small tests
cases = gen(20000, 40)
# medium
cases += gen(3000, 600)
# large multiplication
cases += gen(200, 3000)
cases += gen(40, 20000)
# large division
cases += gen(200, 3000)
# some with one operand tiny
for _ in range(2000):
    a = rand_int(500)
    b = random.choice([1, 2, 3, 9, 10, 99, 100, -1, -2, -7])
    cases.append((a, b))
# explicit edge cases
cases += [
    (0, 5), (0, -5), (5, 1), (-5, 5), (5, -5),
    (-5, -5), (10, 3), (-10, 3), (10, -3), (-10, -3),
    (3, 5), (-3, 5), (3, -5), (-3, -5),
    (1, 1), (-1, 1), (1, -1), (-1, -1),
    (1000000000000000000, 7), (-1000000000000000000, 7),
]

inp = []
for a, b in cases:
    inp.append(f"{a} {b}")
inp_text = "\n".join(inp) + "\n"

res = subprocess.run([BIN], input=inp_text, capture_output=True, text=True)
if res.returncode != 0:
    print("BINARY FAILED")
    print(res.stderr[:2000])
    sys.exit(1)

lines = res.stdout.strip("\n").split("\n")
assert len(lines) == len(cases), f"line count {len(lines)} != {len(cases)}"

fails = 0
for idx, ((a, b), line) in enumerate(zip(cases, lines)):
    parts = line.split(" ")
    exp_s = str(a + b)
    exp_d = str(a - b)
    exp_p = str(a * b)
    exp_q = str(a // b)
    exp_r = str(a % b)
    got = [exp_s, exp_d, exp_p, exp_q, exp_r]
    if parts != got:
        fails += 1
        if fails <= 20:
            print(f"MISMATCH case {idx}: a={a} b={b}")
            print(f"  got: {parts}")
            print(f"  exp: {got}")
print(f"Total cases: {len(cases)}, failures: {fails}")
sys.exit(1 if fails else 0)
