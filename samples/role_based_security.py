# Source code C++ MasterClass (KV Store project) by Fabio Galuppo
# C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
# Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
# May 2025

# python role_based_security.py

import argparse
import redis

parser = argparse.ArgumentParser()
parser.add_argument('--noflush', action='store_true', help='Disable flushing')
parser.add_argument('--db', type=int, choices=range(1, 8), default=3, help='Database number (1–7), default is 3')
args = parser.parse_args()

r = redis.Redis(host='localhost', port=1234, decode_responses=True)
r.select(args.db)
print(f'Database {args.db}')
if not args.noflush:
    r.flushdb()
    print('Flushed')

# --- Setup: Roles and Permissions ---
def setup_roles():
    r.sadd("role:admin", "perms:read", "perms:write", "perms:delete")
    r.sadd("role:editor", "perms:read", "perms:write")
    r.sadd("role:viewer", "perms:read")

# --- Setup: Users and Role Assignments ---
def setup_users():
    r.set("user:alice", "role:admin")
    r.set("user:bob", "role:editor")
    r.set("user:eve", "role:viewer")

# --- Setup: Resources ---
def setup_resources():
    r.zadd("resource:doc1", {"owner:alice": 100})
    r.zadd("resource:doc2", {"owner:bob": 200})

# --- Get role for user ---
def get_user_role(username):
    role_key = r.get(f"user:{username}")
    return role_key if role_key else None

# --- Check if user has permission ---
def has_permission(username, permission):
    role = get_user_role(username)
    if not role:
        return False
    return r.sismember(role, f"perms:{permission}")

# --- Print role and permissions ---
def print_user_permissions(username):
    role = get_user_role(username)
    if not role:
        print(f"{username} has no assigned role.")
        return
    perms = r.smembers(role)
    print(f"{username} has role {role} with permissions:")
    for p in perms:
        print(" -", p.replace("perms:", ""))

# --- Simulate a secure action ---
def attempt_action(user, action):
    print(f"\nAttempting '{action}' by {user}...")
    if has_permission(user, action):
        print(f"[PASS] {user} is allowed to perform '{action}'")
    else:
        print(f"[FAIL] {user} is NOT allowed to perform '{action}'")

setup_roles()
setup_users()
setup_resources()

for user in ["alice", "bob", "eve"]:
    print_user_permissions(user)

# Attempt actions
attempt_action("alice", "delete")
attempt_action("bob", "delete")
attempt_action("eve", "read")
attempt_action("eve", "write")

# Upgrade Eve to editor and retry
print("\nUpgrading Eve to editor...")
r.set("user:eve", "role:editor")
print_user_permissions("eve")
attempt_action("eve", "write")