#include <stdio.h>
#include <stdbool.h> // bool
#include <stdlib.h>  // malloc, free
#include <string.h>  // strtok
#define MAX 100

typedef struct Process {
    int id;                  // 프로세스 id
    int arrive_time;
    int burst_time;
    int deadline;
    int remain_time;         //남은 burst time

    int response_time;       // 응답 시간
    int turnaround_time;     // 소요 시간
    int waiting_time;        // 대기 시간
    bool deadline_satisfied; // false

} Process;

void setProcess(Process *p, int id, int arrive_time, int burst_time, int deadline){
    // 프로세스 구조체를 초기화하는 함수
    p->id = id;
    p->arrive_time = arrive_time;
    p->burst_time = burst_time;
    p->deadline = deadline;
    p->remain_time = burst_time;

    p->response_time = -1;
    p->turnaround_time = -1;
    p->waiting_time = -1;
    p->deadline_satisfied = false;
}

typedef struct priority_queue {
    // 우선순위 큐를 힙을 이용해 구현; 힙은 배열 형태
    Process *Heap[MAX];
    // 힙의 원소 하나는 프로세스 구조체 포인터임
    // main 함수의 프로세스 저장소가 구조체 포인터 배열이기 때문에 공유하기 위함
    int size; // initialize to 0

} Priority_Queue;

Priority_Queue ready_queue; // 레디 큐(우선순위 큐) 선언

void swap(Process *a, Process *b){
    Process tmp = *a;
    *a = *b;
    *b = tmp;
    // 프로세스 배열 내에서 원소 2개의 위치를 서로 바꾸는 함수
}

void push(Process *new){
    // 우선순위 큐에 새로운 원소를 집어넣고, 정렬하는 함수
    if(ready_queue.size+1 > MAX){
        return;
    }

    ready_queue.Heap[ready_queue.size] = new;

    int current = ready_queue.size;
    int parent = (current - 1) / 2;

    while(current > 0 && ready_queue.Heap[current]->deadline < ready_queue.Heap[parent]->deadline){
        swap(ready_queue.Heap[current], ready_queue.Heap[parent]);
        current = parent;
        parent = (current - 1) / 2;
    }

    ready_queue.size ++;
}

Process *pop(void){
    // 우선순위 큐의 root원소를 return하고, 정렬하는 함수
    if(ready_queue.size <= 0){
        return NULL;
    }
    Process *root = ready_queue.Heap[0];

    ready_queue.size --;

    ready_queue.Heap[0] = ready_queue.Heap[ready_queue.size];

    int current = 0;
    int leftChild = current * 2 + 1;
    int rightChild = current * 2 + 2;
    int minNode = current;

    while(leftChild < ready_queue.size){
        if(ready_queue.Heap[minNode]->deadline > ready_queue.Heap[leftChild]->deadline){
            minNode = leftChild;
        }
        if(rightChild < ready_queue.size && ready_queue.Heap[minNode]->deadline > ready_queue.Heap[rightChild]->deadline){
            minNode = rightChild;
        }

        if(minNode == current){
            break;
        } else {
            swap(ready_queue.Heap[current], ready_queue.Heap[minNode]);

            current = minNode;
            leftChild = current * 2 + 1;
            rightChild = current * 2 + 2;
        }
    }

    return root;
}

bool isReadyQueueEmpty(void){
    // 우선순위 큐(ready queue)가 비었는지 확인하는 함수
    if(ready_queue.size == 0){
        return true;
    }
    return false;
}

void end(Process *now, int current_time) {
    // 프로세스가 종료될 경우 처리를 담당하는 함수
    // 프로세스가 종료될 경우, 종료된 시간을 인자로 받아
    // deadline 만족여부, turnaround time, waiting time 계산
    if(current_time <= now->deadline){
        now->deadline_satisfied = true;
    }
    now->turnaround_time = current_time - now->arrive_time;
    now->waiting_time = now->turnaround_time - now->burst_time;
}

int input_data(Process *p[]){
    // 텍스트 파일을 읽어 main의 프로세스 구조체 포인터 배열에 저장하는 함수
    FILE *fd = fopen("process.txt", "r");
    char data[255];
    char *pdata;
    int i = 1;
    int arrive_time, burst_time, deadline;

    while (!feof(fd)) {
        fgets(data, sizeof(data), fd);
        pdata = strtok(data, " ");
        for(int j = 0; j <3; j++){
            if(j == 0){
                arrive_time = atoi(pdata);
                //printf("arrival_time: %d    ", arrive_time);
            }
            else if(j == 1){
                burst_time = atoi(pdata);
                //printf("burst_time: %d    ", burst_time);
            }
            else{
                deadline = atoi(pdata);
                //printf("deadline: %d\n", deadline);
            }
            pdata = strtok(NULL, " ");
        }
        setProcess(p[i-1], i, arrive_time, burst_time, deadline);
        i++;
    }

    fclose(fd);

    return i-1; // i개의 프로세스 (id : 1 ~ i)
}

int main(void){
    int i=0;
    int n;                    // 전체 프로세스 개수
    int current_time = 0;     // 현재 실행 시간
    int p_start_time;         // 지금 실행중인 프로세스 시작 시간
    Process *now;             // 지금 실행중인 프로세스 구조체
    Process *processes[MAX];  // 전체 프로세스를 저장하는 저장소 개념의 구조체 포인터 배열
    double utilization;       // CPU 점유율
    int idle_time = 0;        // 유휴 시간
    bool isNowEmpty = true;   // 현재 실행중인 프로세스가 있는지 확인
    FILE *fp = fopen("result.txt", "w"); //for gantt chart

    ready_queue.size = 0;
    for (i = 0; i < MAX; i++)
    {
        processes[i] = malloc(sizeof(Process));
        ready_queue.Heap[i] = malloc(sizeof(Process));
    }

    /* 텍스트파일 읽기 */
    // 텍스트 파일 읽어 구조체 포인터에 저장
    // 각 구조체 포인터에 대해 메모리 할당
    // 이를 구조체 포인터 배열 processes에 저장
    // n 구하기
    n = input_data(processes);
    fprintf(fp, "%d\n", n);

    /* processes 배열을 arrive_time 순으로 정렬하기(bubble sort)*/
    Process *tmp;
    for (i = 0; i < n - 1; i++){
        for(int j = 0; j < n - (i + 1); j++){
            if(processes[j] -> arrive_time > processes[j+1] -> arrive_time){
                tmp = processes[j];
                processes[j] = processes[j+1];
                processes[j+1] = tmp;
            }
        }
    }
    /*
    printf("process_arrive_time:");
    for(i = 0; i < sizeof(processes)/ sizeof(struct Process *); i++){
        printf("%d ", processes[i]->arrive_time);
    }
    */
    i = 0;
    while(1){
        /* 실행 중인 프로세스도 없고, ready queue가 비고,
        모든 프로세스의 도착시간이 지난 경우 while loop 종료 */
        if(isReadyQueueEmpty() && i>=n && isNowEmpty) {
            break;
        }

        while(i<n && processes[i]->arrive_time <= current_time) { // 여기서 index error 안나게 조건 추가해야함
            // 도착 시간이 같은 프로세스가 여러 개 있을 수 있어 if 대신 while 사용
            //printf("TIME %5d\tID %3d\t ARRIVED\n", current_time, processes[i]->id); // to test
            if(isNowEmpty) { // 실행 중인 프로세스가 없을 경우 바로 실행
                // 구조체 포인터 변수라서 안 값들까지 다 복사
                now = processes[i];
                p_start_time = current_time;
                if(now->response_time == -1) {  // 첫 실행
                    now->response_time = current_time - now->arrive_time;
                }
                isNowEmpty = false;
                //printf("TIME %5d\tPROCESS ID %3d\t START\n", current_time, now->id);
                fprintf(fp, "%d %d ", now->id, current_time);
                // now 시작
            } else if(processes[i]->deadline < now->deadline) { /* 선점 */
                //printf("TIME %5d\tPROCESS ID %3d\t FINISH\n", current_time, now->id);
                fprintf(fp, "%d\n", current_time);
                now->remain_time = now->remain_time - (current_time - p_start_time);
                push(now);
                //printf("TIME %5d\tPROCESS ID %3d\t START\n", current_time, processes[i]->id);
                fprintf(fp, "%d %d ", processes[i]->id, current_time);
                // now 종료, processes[i] 시작
                end(now, current_time);
                now = processes[i];
                p_start_time = current_time;
                if(now->response_time == -1) {  // 첫 실행
                    now->response_time = current_time - now->arrive_time;
                }
            } else {
                push(processes[i]);
            }
            i++;
        }

        if(!isNowEmpty && now->remain_time <= current_time - p_start_time){ // now 프로세스가 끝남
            isNowEmpty = true;
            //printf("TIME %5d\tPROCESS ID %3d\t FINISH\n", current_time, now->id);
            fprintf(fp, "%d\n", current_time);
            // now 종료
            end(now, current_time);
        }

        if(isNowEmpty){
            // 실행 중인 프로세스가 없고 ready queue가 비어있지 않다면 새로운 프로세스 실행
            if(!isReadyQueueEmpty()) {
                now = pop();
                p_start_time = current_time;
                if(now->response_time == -1) {  // 첫 실행
                  now->response_time = current_time - now->arrive_time;
                }
                isNowEmpty = false;
                //printf("TIME %5d\tPROCESS ID %3d\t START\n", current_time, now->id);
                fprintf(fp, "%d %d ", now->id, current_time);
                // now 시작
            } else if(i < n){ // 모든 프로세스가 끝났을 때 세어지는 것 방지
                idle_time ++;
            }
        }

        current_time ++;
    }

    /* utilization 구하기 */
    utilization = (1- (idle_time/(current_time-1))) * 100; //CPU utilization(%) 계산

    /* response time, turnaround time, waiting time, idle time 검증 */
    // process id 순으로 정렬
    for (i = 0; i < n - 1; i++){
        for(int j = 0; j < n - (i + 1); j++){
            if(processes[j] -> id > processes[j+1] -> id){
                tmp = processes[j];
                processes[j] = processes[j+1];
                processes[j+1] = tmp;
            }
        }
    }
    printf("\n");
    float aver_response = 0, aver_turnaround = 0, aver_waiting = 0;
    printf("+-------+--------+----------+-------+\n\
|PROCESS|RESPONSE|TURNAROUND|WAITING|\n\
|   ID  |  TIME  |   TIME   | TIME  |\n\
+-------+--------+----------+-------+\n");
    for(i = 0; i < n; i++){
        printf("|   %3d |", processes[i]->id);
        printf("  %5d |", processes[i]->response_time);
        printf("    %5d |", processes[i]->turnaround_time);
        printf(" %5d |\n", processes[i]->waiting_time);

        aver_response += processes[i]->response_time;
        aver_turnaround += processes[i]->turnaround_time;
        aver_waiting += processes[i]->waiting_time;
    }
    printf("+-------+--------+----------+-------+\n\n");

    aver_response /= n;
    aver_turnaround /= n;
    aver_waiting /= n;

    printf("Total %d Processes\n", n);
    printf("Total Runtime          : %d\n", current_time - 1);
    printf("Utilization            : %.3lf%%\n", utilization);
    printf("Idle Time              : %d\n", idle_time);
    printf("Average Waiting Time   : %.3f\n", aver_waiting);
    printf("Average Turnaround Time: %.3f\n", aver_turnaround);
    printf("Average Response Time  : %.3f\n", aver_response);
    printf("\n");

    /* free memory */
    for(i = 0; i < MAX; i++){
        free(processes[i]);
    }

    return 0;
}
