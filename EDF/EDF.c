#include <stdio.h>
#include <stdbool.h> // bool
#include <stdlib.h>  // malloc, free
#define MAX 100

typedef struct Process {
    int id; // -1            // 프로세스 id
    int arrive_time;
    int deadline;
    int burst_time;

    int response_time;       // 응답 시간
    int turnaround_time;     // 소요 시간
    int waiting_time;        // 대기 시간
    bool deadline_satisfied; // false

} Process;

Process *createProcess(int id, int arrive_time, int deadline, int burst_time){
    Process *new = malloc(sizeof(Process));

    new->id = id;
    new->arrive_time = arrive_time;
    new->deadline = deadline;
    new->burst_time = burst_time;

    new->response_time = -1;
    new->turnaround_time = -1;
    new->waiting_time = -1;
    bool deadline_satisfied = false;

    return new;
}

typedef struct priority_queue {
    Process *Heap[MAX];
    int size = 0;

} Priority_Queue;

void push(Process *new);
Process *pop(void);
void swap(Process *a, Process *b);
bool isReadyQueueEmpty(void);

Priority_Queue ready_queue;

void end(Process *now, current_time) {
    if(current_time <= now->deadline){
    now->deadline_satisfied = true;
    }
    now->turnaround_time = current_time - now->arrive_time;
    now->waiting_time = now->turnaround_time - now->burst_time;
}

int input_data(Process *processes){
    FILE *fd = fopen("input.txt", "r");
    char data[255];
    char *pdata;
    int i=0;

    return i;
}

int main(void){
    int i=0;
    int n;                    // 전체 프로세스 개수
    int current_time = 0;     // 현재 실행 시간
    int p_start_time;         // 지금 실행중인 프로세스 시작 시간
    Process *now;             // 지금 실행중인 프로세스 구조체
    Process *processes[MAX];  // 전체 프로세스를 저장하는 저장소 개념의 프로세스 배열
    double utilization;       // CPU 점유율
    int idle_time = 0;        // 유휴 시간
    bool isNowEmpty = true;   // 현재 실행중인 프로세스가 있는지 확인

    /* 텍스트파일 읽기 */
    // 텍스트 파일 읽어 구조체 포인터에 저장
    // 각 구조체 포인터에 대해 메모리 할당
    // 이를 구조체 포인터 배열 processes에 저장
    // n 구하기
    n = input_data(processes);

    /* utilization 구하기 */

    /* processes 배열을 arrive_time 순으로 정렬하기 */
    // sort(processes, arrive_time);
    // 이건 heap이 아니라 정말 배열을 구조체 변수에 따라 정렬하는 것이기 때문에
    // push, pop에서 사용하는 정렬 함수와 다른 정렬을 사용해야한다.
    // 이때 한번 사용하기 때문에 굳이 함수로 만들지 않아도 되지만 만들면 예쁠것..

    while(1){
        /* 실행 중인 프로세스도 없고, ready queue가 비고,
        모든 프로세스의 도착시간이 지난 경우 while loop 종료 */
        if(isReadyQueueEmpty() && i>=n && isNowEmpty) {
            break;
        }

        while(i<n && processes[i].arrive_time <= current_time) { // 여기서 index error 안나게 조건 추가해야함
            // 도착 시간이 같은 프로세스가 여러 개 있을 수 있어 if 대신 while 사용
            if(isNowEmpty) { // 실행 중인 프로세스가 없을 경우 바로 실행
                // 구조체 포인터 변수라서 안 값들까지 다 복사
                now = processes[i];
                p_start_time = current_time;
                if(now->response_time == -1) {  // 첫 실행
                    now->response_time = current_time;
                }
                isNowEmpty = false;
                // print something
                // now 시작
            } else if(processes[i]->deadline < now->deadline) { /* 선점 */
                now->burst_time = now->burst_time - (current_time - p_start_time);
                push(now);
                // print something
                // now 종료, processes[i] 시작
                end(&now, current_time);
                now = processes[i];
                p_start_time = current_time;
                if(now->response_time == -1) {  // 첫 실행
                    now->response_time = current_time;
                }
            } else {
                push(processes[i]);
            }
            i++;
        }

        if(!isNowEmpty && now->burst_time <= current_time - p_start_time){ // now 프로세스가 끝남
            isNowEmpty = true;
            // print something
            // now 종료
            end(&now, current_time);
        }

        if(isNowEmpty){
            // 실행 중인 프로세스가 없고 ready queue가 비어있지 않다면 새로운 프로세스 실행
            if(!isReadyQueueEmpty()) {
                now = pop();
                p_start_time = current_time;
                if(now->response_time == -1) {  // 첫 실행
                  now->response_time = current_time;
                }
                isNowEmpty = false;
                // print something
                // now 시작
            } else {
                idle_time ++;
            }
        }

        current_time ++;
    }

    return 0;
}
