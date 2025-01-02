# #!/bin/bash

seq_sizes=(50M 100M 150M 200M 250M)
devices=("/dev/sda" "/dev/sdb") 

for device in "${devices[@]}"; do

    device_name=$(basename "$device")
    output_file="dd_${device_name}.txt"
    > "$output_file" 

    for size in "${seq_sizes[@]}"; do
        temp_file="/tmp/${device##*/}-seq-$size" 
        echo "write" >> "$output_file"
        sync; dd if=/dev/zero of="$temp_file" bs="$size" count=1 oflag=direct 2>&1 | tee -a "$output_file" | grep -E 'bytes|time'
        echo "read" >> "$output_file"
        sync; dd if="$temp_file" of=/dev/null bs="$size" 2>&1 | tee -a "$output_file" | grep -E 'bytes|time'
        echo "" >> "$output_file"

        rm "$temp_file"
    done
done

rand_sizes=(1K 5K 10K 15K 20K)

for device in "${devices[@]}"; do
    device_name=$(basename "$device")  
    output_file="fio_${device_name}.txt"
    > "$output_file"  

    for size in "${rand_sizes[@]}"; do
        echo "Testing random write speed on $device with block size: $size" | tee -a "$output_file"
        write_output=$(fio --name=randwrite --ioengine=libaio --filename="/tmp/${device##*/}-seq-$size"  --bs="$size" --size=1G --rw=randwrite --direct=1 --runtime=60 --time_based)
        echo "Random Write IOPS: $write_output" | tee -a "$output_file"
        
        echo "Testing random read speed on $device with block size: $size" | tee -a "$output_file"
        read_output=$(fio --name=randread --ioengine=libaio --filename="/tmp/${device##*/}-seq-$size"  --bs="$size" --size=1G --rw=randread --direct=1 --runtime=60 --time_based)
        echo "Random Read IOPS: $read_output" | tee -a "$output_file"

        echo "" >> "$output_file"
    done
done
