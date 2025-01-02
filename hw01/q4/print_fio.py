import re

output = "q4/fio_sda.txt"
write_latencies = []
read_latencies = []

with open(output, 'r') as file:
        c = ""
        for line in file:
            if "Testing" in line:
                if "write" in line:
                    c="w"
                elif "read" in line:
                    c="r"
            if "   lat (usec):" in line:
                a = line.find("avg=")
                b = line[a:].find(",")
                time=line[a+4:a+b]
                if c == "w":
                    write_latencies.append(float(time))
                elif c == "r":
                    read_latencies.append(float(time))

            
print(read_latencies, write_latencies)
# Regular expressions to find average latencies
# write_lat_pattern = r'lat \(usec\): .*?avg=(\d+\.\d+)'
# read_lat_pattern = r'lat \(usec\): .*?avg=(\d+\.\d+)'

# # Find all matches for write latencies
# write_latencies = [float(match) for match in re.findall(write_lat_pattern, output)]
# read_latencies = [float(match) for match in re.findall(read_lat_pattern, output)]

# print(write_latencies, read_latencies)

# # Calculate averages
avg_write_latency = sum(write_latencies) / len(write_latencies) / 1000000 if write_latencies else 0
avg_read_latency = sum(read_latencies) / len(read_latencies) / 1000000 if read_latencies else 0

print(f"Average Write Latency: {avg_write_latency} sec")
print(f"Average Read Latency: {avg_read_latency} sec")
