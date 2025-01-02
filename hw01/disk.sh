#!/bin/bash

# 1. Read details about the hard disk
total_size=$(df --output=size -h / | tail -1)
free_space=$(df --output=avail -k / | tail -1)

echo "Total Size: $total_size"
echo "Free Space: $free_space"

# 2. Take input - sentence + name of file
echo "Enter a sample sentence:"
read sample_sentence

echo "Enter a file name"
read file_name

# 3. Create the file with several instances of the sentence
touch $file_name

instances=$((($free_space / 100000)))

for i in $(seq 1 $instances)
do
    echo "$sample_sentence" >> "$file_name"
done

echo "File '$file_name' created with $instances instances of the '$sample_sentence'"
