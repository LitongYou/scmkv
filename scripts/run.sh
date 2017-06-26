#!/bin/bash

echo $*

nthreads=1
while [ $nthreads != "4" ] ; 
do
    echo "============================"
    echo "start with $nthreads threads"
    $* -a $nthreads
    ((nthreads++))
done

