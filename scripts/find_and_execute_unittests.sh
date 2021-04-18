#!/usr/bin/env bash

Failed_cases=()
Failed=0
for test in $(find unittests -executable -type f); do
    $test

    if [ $? -ne 0 ]; then
        Failed_cases+=("$test")
        Failed=1
    fi
done

if [ $Failed -ne 0 ]; then
    echo "****** The following cases did not pass:"
    for fc in  "${Failed_cases[@]}"; do
        echo "- ${fc}"
    done

    exit 1
fi

exit 0
