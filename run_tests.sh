#!/bin/sh

set -eu

rc=0

make debug

for input in test/*.in
do
    name=$(basename "$input" | cut -f 1 -d '.')
    expected="test/$name.out"
    actual="test/$name.tmp"

    printf "%s " "$name"
    MALLOC_OPTIONS=CFGJSUX ./bin/debug/pop "$@" "$input" > "$actual"
    if ! cmp -s "$expected" "$actual"
    then
        echo "fail"
        diff -u "$expected" "$actual"
        rc=1
    else
        echo "ok"
    fi

    rm "$actual"
done

exit $rc
