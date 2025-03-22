import matplotlib.pyplot as plt

FILE_PATH = 'wave.pcm.txt'

with open(FILE_PATH, 'r') as file:
    y_values = [float(line.strip()) for line in file]

# Generate x values evenly spaced
x_values = [i / (len(y_values) - 1) for i in range(len(y_values))]

# Plot the data
plt.figure(figsize=(10, 6))
plt.plot(x_values, y_values, marker='o', markersize=2, linestyle='-', color='b')

# Add labels and title
plt.title('Plot of PCM frames')
plt.grid(True)
plt.show()
