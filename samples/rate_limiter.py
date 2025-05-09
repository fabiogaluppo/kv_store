# Source code C++ MasterClass (KV Store project) by Fabio Galuppo
# C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
# Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
# May 2025

# python rate_limiter.py

import argparse
import time
from datetime import datetime
import uuid
import redis

parser = argparse.ArgumentParser()
parser.add_argument('--noflush', action='store_true', help='Disable flushing')
parser.add_argument('--db', type=int, choices=range(1, 8), default=1, help='Database number (1–7), default is 1')
args = parser.parse_args()

r = redis.Redis(host='localhost', port=1234, decode_responses=True)
r.select(args.db)
print(f'Database {args.db}')
if not args.noflush:
    r.flushdb()
    print('Flushed')

class RateLimiter:
    def __init__(self, rcli, max_calls_in_period, time_period_in_seconds):
        self.rcli = rcli
        self.max_calls_in_period = max(1, max_calls_in_period)
        self.time_period_in_seconds = max(1, time_period_in_seconds)
    def allow(self, key, req_id):
        current_time = int(time.time())
        self.rcli.zremrangebyscore(key, 0, current_time - self.time_period_in_seconds)
        if self.rcli.zcard(key) < self.max_calls_in_period:
            self.rcli.zadd(key, {req_id: current_time})
            return True
        return False

def test_call_limit(n, client_id):
    for i in range(n):
        req = 'req-' + str(uuid.uuid4())
        now = datetime.now()
        tm = now.strftime("%H:%M:%S.") + f"{now.microsecond // 1000:03d}"
        print(f"[{tm}] {req} for {client_id}", end=' ') 
        if rate_limiter.allow(client_id, req):
            print("allowed")
        else:
            print("not allowed")

rate_limiter = RateLimiter(r, 5, 3)
test_call_limit(12, 'client1')
test_call_limit(12, 'client2')
time.sleep(3)
test_call_limit(12, 'client1')
test_call_limit(12, 'client2')