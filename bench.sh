#!/bin/bash
clear
make bench

for i in stl_fail_bench.exe pre_fail_bench.exe stl_pass_bench.exe pre_pass_bench.exe; do
    echo $i
    for j in 1 2 3; do
	time ./$i
    done
done

exit 0
    
