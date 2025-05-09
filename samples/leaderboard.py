# Source code C++ MasterClass (KV Store project) by Fabio Galuppo
# C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
# Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
# May 2025

# python leaderboard.py

import argparse
import redis

parser = argparse.ArgumentParser()
parser.add_argument('--noflush', action='store_true', help='Disable flushing')
parser.add_argument('--db', type=int, choices=range(1, 8), default=4, help='Database number (1–7), default is 4')
args = parser.parse_args()

r = redis.Redis(host='localhost', port=1234, decode_responses=True)
r.select(args.db)
print(f'Database {args.db}')
if not args.noflush:
    r.flushdb()
    print('Flushed')

# Select database
r.select(1)

# Flush all data for a clean start (use with caution in real systems)
r.flushdb()

# --- Leaderboard setup with ZADD ---
r.zadd("leaderboard", {"alice": 1500, "bob": 1200, "carol": 1800, "dave": 2000})

# --- Get ZSCORE for a player ---
print("Alice's score:", r.zscore("leaderboard", "alice"))

# --- Get top players (ZRANGE WITHSCORES) ---
top_players = r.zrange("leaderboard", 1200, 2000, withscores=True, byscore=True)
print("Top players:")
for name, score in reversed(top_players):
    print(f"  {name}: {score}")

# --- Add players to teams using SADD ---
r.sadd("team:red", "alice", "bob")
r.sadd("team:blue", "carol", "dave")

# --- List all team members (SMEMBERS) ---
print("Red team members:", [m for m in r.smembers("team:red")])
print("Blue team members:", [m for m in r.smembers("team:blue")])

# --- Count players in a team (SCARD) ---
print("Red team size:", r.scard("team:red"))

# --- Check if player is in a team (SISMEMBER) ---
print("Is Bob in Red team?", bool(r.sismember("team:red", "bob")))

# --- Players in both teams (SINTER) ---
print("Players in both teams:", [m for m in r.sinter("team:red", "team:blue")])

# --- All unique players in both teams (SUNION) ---
print("All players in any team:", [m for m in r.sunion("team:red", "team:blue")])

# --- Remove a player from a team (SREM) ---
r.srem("team:red", "bob")
print("Red team after removing Bob:", [m for m in r.smembers("team:red")])

# --- Delete a player (DEL + ZREM) ---
r.delete("player:alice")  # Suppose we had some player metadata
r.zrem("leaderboard", "alice")
print("Leaderboard after removing Alice:", r.zrange("leaderboard", 0, -1, withscores=True))

# --- Remove low scores (ZREMRANGEBYSCORE) ---
r.zremrangebyscore("leaderboard", 0, 1300)
print("Leaderboard after removing low scores:", r.zrange("leaderboard", 0, -1, withscores=True))

# --- Check if keys exist (EXISTS) ---
print("Leaderboard exists?", bool(r.exists("leaderboard")))
print("Red team exists?", bool(r.exists("team:red")))

# --- List all keys (KEYS) ---
print("All team keys:", [k for k in r.keys("team:*")])
print("All keys:", [k for k in r.keys("*")])