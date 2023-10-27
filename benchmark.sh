#!/usr/bin/env bash

make clean
make

for test in "tests"/*
do
    echo "Benchmarking $test"
    hyperfine --warmup 3 --shell=none -L runmode i,j "./bfi -{runmode} $test" --export-markdown "benchmarks/benchmark-$(basename "$test").md"
    echo "-----------------------------------------------------------"
done
