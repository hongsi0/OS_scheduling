import matplotlib.pyplot as plt
import numpy as np
from bokeh.palettes import Turbo256, turbo

# 바 크기 지정
PROC_HEIGHT = 15
PROC_SPACING = 10

# 프로세스마다의 바 중간의 위치 계산 
def bar_mid(number) :
    return (number * PROC_SPACING) + ((number - 1) * PROC_HEIGHT) + (PROC_HEIGHT/2)

# 프로세스마다의 바 밑의 위치 계산
def bar_bottom(number) :
    return (number * PROC_SPACING) + ((number - 1) * PROC_HEIGHT)

# 간트차트 출력 함수
def scheduling_plot(result):

    # 스케줄링 결과 txt파일 읽기
    with open(result, 'rt') as file:
        data = file.readlines()

    # 공백 제거 
    data = [i.strip() for i in data]

    # 프로세스 개수 num 변수에 저장
    num = data[0].split()
    num = int(num[0])
    
    # 랜덤 색상 팔레트 생성
    colors = [''] # colors[0]은 사용하지 않음
    r = lambda: np.random.randint(0,255)
    for i in range(1, num+1):
        colors.append('#%02X%02X%02X' % (r(),r(),r()))

    # 프로세스 개수 data 배열에서 삭제
    del data[0]

    start = 0

    # 전체 종료 시간 endTime 변수에 저장
    end = len(data)-1
    tmp = data[end].split()
    endTime = int(tmp[2])

    # plot 생성, 프로세스 개수와 종료시간에 따라 크기 지정 및 label 설정     
    fig, gantt = plt.subplots(figsize = (endTime * 0.5, num * 1.5))

    gantt.set_xlim(0, endTime+1)
    gantt.set_ylim(0, ((PROC_HEIGHT * num) + (PROC_SPACING * (num + 1))))

    gantt.set_xlabel("Time")
    gantt.set_ylabel("Process")

    yticks = [None] * num
    yticklabels = [None] * num

    for i in range(num) :
        yticks[i] = bar_mid(i + 1)
        yticklabels[i] = i+1

    gantt.set_yticks(yticks)
    gantt.set_yticklabels(yticklabels)
    
    xticks = [0] * (endTime + 1)
    xticklabels = [None] * (endTime + 1)
    
    for i in range(endTime + 1) :
        xticks[i] = i 
        xticklabels[i] = i
        
    gantt.set_xticks(xticks)
    gantt.set_xticklabels(xticklabels)

    gantt.grid(True)

    # 각 프로세스마다의 바 출력 
    for i in data:
        data = i.split()
        execute_time = int(data[2]) - int(data[1])
        gantt.broken_barh([(int(data[1]), execute_time)], (bar_bottom(int(data[0])), (PROC_HEIGHT)), color=turbo(num)[int(data[0])-1])
        
    # 공백 줄이기
    plt.tight_layout()
    
    # 결과 출력 파일로 저장 시 주석 처리 필요
    # plt.show()

    # png 파일로 결과 저장
    plt.savefig("gantt_chart.png")
    
# 함수 호출
scheduling_plot("result.txt")
