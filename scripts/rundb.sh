#!/bin/bash

parameters=("-c 40 -k 16 -v 100 -g u -e w"
"-c 40 -k 16 -v 100 -g u -e r"
"-c 40 -k 16 -v 100 -g z -e w"
"-c 40 -k 16 -v 100 -g z -e r"
"-c 8 -k 128 -v 512 -g u -e w"
"-c 8 -k 128 -v 512 -g u -e r"
"-c 8 -k 128 -v 512 -g z -e w"
"-c 8 -k 128 -v 512 -g z -e r"
)

for pm in "${parameters[@]}"
do
    echo "*******************************************************"
    echo "start with new parameters"
    echo "$* $pm"
    nthreads=1
    while [ $nthreads != "17" ] ; 
    do
        echo "================================================"
        echo "start with $nthreads threads"
        $* $pm -a $nthreads
        ((nthreads++))
    done
    echo ""
    echo "end with these parameters"
    echo ""
    echo ""
done

