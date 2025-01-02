import re
import matplotlib.pyplot as plt

hd_dd = "q4/dd_sda.txt"
usb_dd = "q4/dd_sdb.txt"
hd_fio = "q4/fio_sda.txt"
usb_fio = "q4/fio_sdb.txt"

def extract_times_ran(file_path):
    read_latencies = [] 
    write_latencies = []
    with open(file_path, 'r') as file:
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
                    write_latencies.append(float(time)/1000000)
                elif c == "r":
                    read_latencies.append(float(time)/1000000)

    return read_latencies, write_latencies
   
def extract_times_seq(file_path):
    seq_read_times = []
    seq_write_times = []
    time_pattern = re.compile(r'(\d+\.\d+) s')

    with open(file_path, 'r') as file:
        c = ""
        for line in file:
            if ("write" in line):
                c = "sw"
            elif ("read" in line):
                c = "sr"

            if "copied" in line:
                a = line.find("copied, ")
                k = line[a:]
                b = k.find(" ")
                time=line[a+8:a+8+b+1]
                time = float(time)
                if c == "sw":
                    seq_write_times.append(time)
                elif c == "sr":
                    seq_read_times.append(time)
                
    return seq_read_times, seq_write_times

sda_seq_read, sda_seq_write = extract_times_seq(hd_dd)
sdb_seq_read, sdb_seq_write = extract_times_seq(usb_dd)
sda_ran_read, sda_ran_write = extract_times_ran(hd_fio)
sdb_ran_read, sdb_ran_write = extract_times_ran(usb_fio)

def calculate_mean(times):
    return sum(times) / len(times) if times else 0

mean_sda_seq_read = calculate_mean(sda_seq_read)
mean_sda_seq_write = calculate_mean(sda_seq_write)
mean_sda_rand_read = calculate_mean(sda_ran_read)
mean_sda_rand_write = calculate_mean(sda_ran_write)

mean_sdb_seq_read = calculate_mean(sdb_seq_read)
mean_sdb_seq_write = calculate_mean(sdb_seq_write)
mean_sdb_rand_read = calculate_mean(sdb_ran_read)
mean_sdb_rand_write = calculate_mean(sdb_ran_write)

test_names = ['Sequential Read', 'Sequential Write', 'Random Read', 'Random Write']
sda_means = [mean_sda_seq_read, mean_sda_seq_write, mean_sda_rand_read, mean_sda_rand_write]
sdb_means = [mean_sdb_seq_read, mean_sdb_seq_write, mean_sdb_rand_read, mean_sdb_rand_write]

bar_width = 0.25
index = range(len(test_names))

plt.figure(figsize=(10, 6))

plt.bar(index, sda_means, bar_width, label='/dev/sda', color='blue')
plt.bar([i + bar_width for i in index], sdb_means, bar_width, label='/dev/sdb', color='orange')

plt.xlabel('Test Type')
plt.ylabel('Mean Time (s)')
plt.title('Mean Time Taken for Random and Sequential Reads and Writes on /dev/sda and /dev/sdb')
plt.xticks([i + bar_width / 2 for i in index], test_names)

for i, v in enumerate(sda_means):
    plt.text(i, v + 0.001, f"{v:.4f}", ha='center')
for i, v in enumerate(sdb_means):
    plt.text(i + bar_width, v + 0.001, f"{v:.4f}", ha='center')

plt.legend()
plt.tight_layout()
plt.show()
