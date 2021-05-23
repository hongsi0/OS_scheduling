import matplotlib.pyplot as plt
import numpy as np

TASK_HEIGHT = 6
TASK_SPACING = 10


def task_mid(number) :
    return (number * TASK_SPACING) + ((number - 1) * TASK_HEIGHT) + (TASK_HEIGHT/2)

def task_bottom(number) :
    return (number * TASK_SPACING) + ((number - 1) * TASK_HEIGHT)

def scheduling_plot(result):
    with open(result, 'rt') as file:
        data = file.readlines()

    data = [i.strip() for i in data]

    num = data[0].split()
    num = int(num[0])

    del data[0]

    start = 0

    #endTime -= 1

    #for i in reversed(data):
     #   data = i.split()
     #   print(data)
     #   if (int(data[0]) == endTime):
     #       end = data.index(i)
     #       break
    #print(end)

    #if end == 0:
        #end = len(data) - 1
        #endTime = data[end].split()
        #endTime = int(endTime[2])


    #del data[end:(len(data)-1)]

    fig, gantt = plt.subplots(figsize = (10, num * 1.5))
    end = len(data)-1
    tmp = data[end].split()
    endTime = int(tmp[2])

    gantt.set_xlim(0, endTime+1)
    gantt.set_ylim(0, ((TASK_HEIGHT * num) + (TASK_SPACING * (num + 1))))

    gantt.set_xlabel("Time")
    gantt.set_ylabel("Process")

    yticks = [None] * num
    yticklabels = [None] * num

    for i in range(num) :
        yticks[i] = task_mid(i + 1)
        yticklabels[i] = i+1

    gantt.set_yticks(yticks)
    gantt.set_yticklabels(yticklabels)

    # gantt.grid(axis = 'x')

    for i in data:
        data = i.split()
        for j in range(1, num + 1) :
            if ((int(data[0]) == j)):
                execute_time = int(data[2]) - int(data[1])
                gantt.broken_barh([(int(data[1]), execute_time)], (task_bottom(j),10), color ='black')

    plt.tight_layout()

    plt.savefig("gantt_chart.png")
    plt.show()

scheduling_plot("result.txt")
