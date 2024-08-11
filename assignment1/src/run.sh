#!/bin/bash

for i in {1..7} 
do 
    echo "Running iteration $i for image $i"
    
    for j in {1..5}  
    do 
        INPUT_IMAGE=$i 
        OUTPUT_IMAGE=${INPUT_IMAGE}_answer_${j}_time  
        make run-sharpen INPUT=$INPUT_IMAGE OUTPUT=$OUTPUT_IMAGE
    done
    
    echo ""
done 

echo "All iterations completed."