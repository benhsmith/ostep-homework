import sys
import csv
import matplotlib.pyplot as plt

f = open(sys.argv[1])
reader = csv.reader(f)

x = []
ticks = []
y = []

i = 0

for line in reader:
    ticks.append(line[0])
    x.append(i)
    y.append(int(line[1]))

    i = i + 1

plt.plot(x, y)
plt.xticks(x, ticks)
plt.show()
