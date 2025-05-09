# Source code C++ MasterClass (KV Store project) by Fabio Galuppo
# C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
# Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
# May 2025

# python perf_strings.py --n=100000 --db=2 --noflush
# python perf_strings.py --n=100000 --db=5 --noflush

import argparse
import time
import redis

def ensure_n(value):
    val = int(value)
    if val < 1 or val > 1_000_000:
        raise argparse.ArgumentTypeError("n must be between 1 and 1 million")
    return val

parser = argparse.ArgumentParser()
parser.add_argument('--noflush', action='store_true', help='Disable flushing')
parser.add_argument('--db', type=int, choices=range(1, 8), default=2, help='Database number (1–7), default is 2')
parser.add_argument('--n', type=ensure_n, default=10_000, help='Test size (1–1M), default is 10K')
args = parser.parse_args()

r = redis.Redis(host='localhost', port=1234, decode_responses=True)
r.select(args.db)
print(f'Database {args.db}')
if not args.noflush:
    r.flushdb()
    print('Flushed')

start = time.perf_counter()
print(f'Inserting {args.n} pairs of keys and values...')
for i in range(args.n):
    r.set(f'key{i}', f'val{i}')
end = time.perf_counter()
print("Insertion Completed!", end=' ')
print(f"Elapsed time: {end - start:.6f} seconds")

start = time.perf_counter()
print(f'Removing {args.n} keys...')
for i in range(args.n):
    r.delete(f'key{i}')
end = time.perf_counter()
print("Removal Completed!", end=' ')
print(f"Elapsed time: {end - start:.6f} seconds")