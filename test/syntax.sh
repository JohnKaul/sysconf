#!/bin/sh

fail=0
cntr=1

for test_in in test/syntax/*.in; do
    # Create a temporary copy of the config file so a diff can be run
    # on it after inner loop.
    tmp=test/syntax/$(basename ${test_in} .in).tmp
    cp "$test_in" "$tmp"

    for test_cmd in test/syntax/$(basename ${test_in} .in)_*.cmd; do
        cat $test_in | ./sysconf -f ${test_in} $(cat $test_cmd) > test.out
        e=$?
        if [ $e -gt 0 ]; then
            printf "[%02d] FAIL : %s\n" $cntr $(basename $test_cmd .cmd)
            fail=$(( $fail + 1 ))
        else
            res=$(diff -u test/syntax/$(basename $test_cmd .cmd).res test.out)
            if [ $? -gt 0 ]; then
                printf "[%02d] FAIL : %s\n" $cntr $(basename $test_cmd .cmd)
                printf '%s' "$res" | sed 's/^/    /'
                fail=$(( $fail + 1 ))
            else
                printf "[%02d] PASS : %s\n" $cntr $(basename $test_cmd .cmd)
            fi
        fi
        cntr=$(( $cntr + 1 ))
    done

    # Run a diff on input config file; hopefully we've run the tests
    # to restore the config file back to it's orig state.
    res=$(diff -u $tmp $test_in)
    if [ $? -gt 0 ]; then
        printf "[%02d] FAIL : %s\n" $cntr "Config File difference: $test_in"
    else
        printf "[%02d] PASS : %s\n" $cntr "Config file clean: $test_in"
    fi
    rm $tmp
    cntr=$(( $cntr + 1 ))
done

printf "1..%d\n" $(( $cntr - 1 ))
rm test.out

exit $fail
