#!/bin/sh
#
# Verify that getopt accepts CLI options in any order.
#
# Options under test: -f <file>, -n, -d <file>
# Non-option arguments (key, key=value) must still follow all option flags.

fail=0
cntr=1

config=test/syntax/get.in

pass() {
    printf "[%02d] PASS : %s\n" "$cntr" "$1"
    cntr=$(( cntr + 1 ))
}

fail() {
    printf "[%02d] FAIL : %s\n" "$cntr" "$1"
    if [ -n "$2" ]; then
        printf "    expected: %s\n" "$2"
        printf "    actual:   %s\n" "$3"
    fi
    fail=$(( fail + 1 ))
    cntr=$(( cntr + 1 ))
}

check() {
    label="$1"
    expected="$2"
    actual="$3"
    rc="$4"

    if [ "$rc" -ne 0 ]; then
        fail "$label (exit $rc)"
    elif [ "$actual" != "$expected" ]; then
        fail "$label" "$expected" "$actual"
    else
        pass "$label"
    fi
}

# -f first (baseline)
out=$(./sysconf -f "$config" key1 2>&1); rc=$?
check "getopt_f_first" "value1 " "$out" "$rc"

# -n before -f
out=$(./sysconf -n -f "$config" key1 2>&1); rc=$?
check "getopt_n_before_f" "key1: value1 " "$out" "$rc"

# -n after -f (but before positional arg)
out=$(./sysconf -f "$config" -n key1 2>&1); rc=$?
check "getopt_n_after_f" "key1: value1 " "$out" "$rc"

# -f only (no positional — print all); just verify exit 0
./sysconf -f "$config" > /dev/null 2>&1; rc=$?
check "getopt_f_only_exit_ok" "" "" "$rc"

# -d before -f vs -f before -d: both orderings must exit 0 and produce identical output
out_d_first=$(./sysconf -d "$config" -f "$config" 2>&1); rc=$?
check "getopt_d_before_f_exit_ok" "" "" "$rc"

out_f_first=$(./sysconf -f "$config" -d "$config" 2>&1); rc=$?
check "getopt_f_before_d_exit_ok" "" "" "$rc"

if [ "$out_d_first" = "$out_f_first" ]; then
    pass "getopt_d_f_order_same_output"
else
    fail "getopt_d_f_order_same_output" "$out_d_first" "$out_f_first"
fi

# -n -f -d (all three flags, -n is irrelevant in -d mode but must not error)
./sysconf -n -f "$config" -d "$config" > /dev/null 2>&1; rc=$?
check "getopt_n_f_d_exit_ok" "" "" "$rc"

# -d -n -f (reverse of above; same exit code expected)
./sysconf -d "$config" -n -f "$config" > /dev/null 2>&1; rc=$?
check "getopt_d_n_f_exit_ok" "" "" "$rc"

printf "1..%d\n" $(( cntr - 1 ))
exit "$fail"
