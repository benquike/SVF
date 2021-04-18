#!/bin/sh

for test in `find unittests -executable -type f`; do
    $test
done
