import pandas as pd
import matplotlib.pyplot as plt

# Read fio results
results_file = 'fio_results.txt'

# Read the file and extract the relevant data
data = pd.read_csv(results_file, sep=' ', skiprows=1, header=None)
data.columns = ['Name', 'IOPS', 'BW(MiB/s)', 'Lat(ms)', 'Error']

# Filter for the relevant tests
hd_seq_read = data[(data['Name'] == 'seq_read') & (data['Device'] == 'hard_drive')]
usb_seq_read = data[(data['Name'] == 'seq_read') & (data['Device'] == 'usb')]
hd_rand_read = data[(data['Name'] == 'rand_read') & (data['Device'] == 'hard_drive')]
usb_rand_read = data[(data['Name'] == 'rand_read') & (data['Device'] == 'usb')]

# Calculate mean time taken (in milliseconds)
mean_times = [
    hd_seq_read['Lat(ms)'].mean(),
    usb_seq_read['Lat(ms)'].mean(),
    hd_rand_read['Lat(ms)'].mean(),
    usb_rand_read['Lat(ms)'].mean()
]

# Prepare labels
test_names = ['HD Seq Read', 'USB Seq Read', 'HD Rand Read', 'USB Rand Read']

# Create the bar graph
plt.figure(figsize=(10, 6))
plt.bar(test_names, mean_times, color=['blue', 'green', 'orange', 'red'])

plt.title('Mean Time Taken for Reads')
plt.xlabel('Test Type')
plt.ylabel('Mean Time (ms)')
plt.ylim(0, max(mean_times) + 10)  # Set y-axis limit for better visualization
plt.grid(axis='y')

# Show the values on top of the bars
for i, v in enumerate(mean_times):
    plt.text(i, v + 0.5, f"{v:.2f}", ha='center')

plt.show()
