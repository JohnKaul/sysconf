#!/bin/sh

fail=0
cntr=1

for test_in in test/syntax/*.in; do
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
done
printf "1..%d\n" $(( $cntr - 1 ))
rm test.out

exit $fail
