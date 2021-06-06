#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESS 100
#define FILE_NAME "process.txt"

int proc_num;   // 총 프로세스 갯수
int curr_time = 0;  // 현재 running time
int context_switch = 1; // context switch 횟수
int prev_proc=-1;   // 이전에 실행되었던 프로세스의 process id
int total_burst_time = 0;   // 총 burst time, ready queue에 있는 프로세스의 burst time을 저장시키기 위함
int TQ = 0; // time quantum
double idle_time = 0;
double utilization;


struct _process{
    int process_id; // 프로세스 id
    float arrival_time; // 실행 뒤 ready queue에 들어오는 도착 시간
    int r_at;   // 프로세스가 처음으로 ready queue에 들어오는 도착 시간
    int burst_time; // 프로세스의 burst time
    int waiting_time;   // 프로세스의 waiting time
    int response_time;  // 프로세스의 response time
    int turnaround_time;    // 프로세스의 turnaround time
    int remain_time;    // 실행 뒤 남은 burst time
};

// 도착 시간에 따른 정렬
void sort(struct _process *proc, int total_proc) {
    struct _process temp;
    for(int i = 0 ; i < total_proc ; i++) { //
        for(int j = i + 1 ; j < total_proc ; j++) {
            if(proc[i].arrival_time > proc[j].arrival_time) { // 도착 시간이 작은 순으로 정렬
                temp = proc[i];
                proc[i] = proc[j];
                proc[j] = temp;
            }
        }
    }
}

// 남은 burst time에 따른 ready queue 정렬
// 2번째 인자로 total process의 갯수가 아닌 ready queue에 있는 프로세스의 갯수만 정렬을 함
void sort_bt(struct _process *proc, int rq_proc) {
    struct _process temp;
    for(int i = 0 ; i < rq_proc; i++) {
        for(int j = i + 1 ; j < rq_proc; j++) {
            if(proc[i].remain_time > proc[j].remain_time) { // 남은 burst time이 작은 순으로 정렬
                temp = proc[i];
                proc[i] = proc[j];
                proc[j] = temp;
            }
        }
    }
}

// 프로세스 데이터 입력
int input_data(struct _process *proc){
    FILE *fd = fopen(FILE_NAME, "r"); // 텍스트 파일 open
    char data[255]; // 텍스트 파일의 한 줄을 저장할 공간 선언
    char *pdata;    // data의 포인터
    int i = 0;      // 총 프로세스의 갯수

    while(!feof(fd)){   // 파일의 마지막이 아닐 경우 while문 조건 부합
        fgets(data, sizeof(data), fd);  // 텍스트 파일의 한줄을 data에 저장
        pdata = strtok(data, " ");      // data에 저장된 값을 첫 번째 띄어쓰기가 나오기 전까지의 단어를 pdata 포인터가 가리킴
        for(int j = 0; j < 3; j++){     // process id, arrival time, burst time을 받기 때문에 3번의 반복을 통해 해당 값들을 가져옴
            if(j==0){
                proc[i].process_id = atoi(pdata);
            }
            else if(j==1){
                proc[i].arrival_time = atoi(pdata);
                proc[i].r_at = atoi(pdata);
            }
            else{
                proc[i].burst_time = atoi(pdata);
                proc[i].remain_time = atoi(pdata);
                total_burst_time = total_burst_time + atoi(pdata);  // 초기 time quantum을 모든 프로세스의 burst time 평균으로 설정해 주기 위함
            }
            pdata = strtok(NULL, " ");
        }
        i++;    // 프로세스의 갯수를 count
    }
    fclose(fd);

    return i;
}

// context switch 발생 횟수
// 첫번째 인자 -> struct process, 두번째 인자 -> 현재 실행중인 프로세스의 index
void contswitch_count(struct _process *proc, int rq_index){
    if(curr_time > 0 && prev_proc != proc[rq_index].process_id){ // 이전에 실행된 프로세스가 존재하고 이전에 실행된 프로세스와 현재 실행중인 프로세스가 다르다면
        context_switch++;   // context switch 횟수 count
    }
}

// 처음 시작되는 프로세스의 도착 시간이 0이 아닌 경우 시작 시간 설정
void check_procstart(struct _process *proc){
    if(proc[0].arrival_time > curr_time){   // 처음 실행되는 프로세스의 도착 시간이 0보다 크다면
        curr_time = curr_time + proc[0].r_at;   // 시작 시간에 처음 실행되는 프로세스의 도착 시간을 반영
    }
}

// 프로세스 response time 산출
// 첫번째 인자 -> struct process, 두번째 인자 -> 현재 실행중인 프로세스의 index
void cal_response_time(struct _process *proc, int rq_index){
    if(proc[rq_index].burst_time == proc[rq_index].remain_time){ // 현재 실행중인 프로세스의 총 burst time과 남은 burst time이 같다면
        proc[rq_index].response_time = curr_time-proc[rq_index].r_at;   // 현재 프로세스의 response time = 현재까지 실행 시간 - 프로세스의 도착 시간
    }
}

// time quantum 내에 프로세스가 끝날 경우 남은 burst time / turnaround time / waiting time / arrival time 설정 및 현재 running time 산출
void proc_burst_end(struct _process *proc, int rq_index){
    cal_response_time(proc, rq_index);  // response time 산출
    curr_time = curr_time + proc[rq_index].remain_time; // time quantum 내에 프로세스가 종료되기 때문에 현재까지 실행 시간 = 현재 시간 + 프로세스의 남은 burst time
    proc[rq_index].remain_time = 0; // 프로세스의 남은 burst time을 0으로 설정
    proc[rq_index].turnaround_time = curr_time - proc[rq_index].r_at;   // 프로세스의 turnaround time = 현재까지 실행 시간 - 프로세스의 처음 도착 시간
    proc[rq_index].waiting_time = proc[rq_index].turnaround_time - proc[rq_index].burst_time;   // 프로세스의 waiting time = 프로세스의 turnaround time - 프로세스의 burst time
    proc[rq_index].arrival_time = 100000000;    // 프로세스가 종료되었기 때문에 배열의 마지막에 배치하기 위함
}

// time quantum 내에 프로세스가 끝나지 않을 경우 남은 burst time / arrival time 설정 및 현재 running time 산출
// 첫번째 인자 -> struct process, 두번째 인자 -> 현재 실행중인 프로세스의 index
void proc_burst_remain(struct _process *proc, int rq_index){
    cal_response_time(proc, rq_index);  // response time 산출
    proc[rq_index].remain_time = proc[rq_index].remain_time - TQ;   // time quantum내에 프로세스가 종료되지 않기 때문에 프로세스의 남은 burst time = 프로세스 남은 burst time - time quantum
    curr_time = curr_time + TQ; // 현재까지 실행 시간 = 현재까지 실행 시간 + time quantum
    proc[rq_index].arrival_time = curr_time+0.00000001; // 같은 시간대에 들어오는 프로세스와 남은 burst time이 같은 경우 현재 실행중인 프로세스의 나중에 실행되도록 설정하기 위함
}

// ready queue에 있는 프로세스 갯수 및 ready queue에 있는 프로세스의 총 burst time 산출
// 첫번째 인자 -> struct process, 두번째 인자 -> 현재까지 실행 시간
int cal_rqnum_tbt(struct _process *proc, int __curr_time){
    int proc_count = 0;
    total_burst_time = 0;

    for(int i=0; i<proc_num; i++){   // 도착 시간이 현재까지 실행 시간보다 작은 프로세스의 갯수를 count
        if(proc[i].arrival_time > __curr_time+0.1){
            break;
        }

        total_burst_time = total_burst_time + proc[i].remain_time;
        proc_count++;
    }
    return proc_count;
}

// 각 프로세스의 process id / waiting time / turnaround time / response time 및 context switch / 총 runnign time /
// average of waiting time / average of turnaround time / average of response time 출력
void print_result(struct _process *proc){
    float total_wt = 0;
    float total_tat = 0;
    float total_rpt = 0;
    // double utilization = (1- (idle_time/(current_time-1))) * 100;

    for(int i=0; i < proc_num; i++){
        total_wt = total_wt + proc[i].waiting_time;
        total_tat = total_tat + proc[i].turnaround_time;
        total_rpt = total_rpt + proc[i].response_time;

        if(i==0){
            printf("\n\
+-------+--------+----------+-------+\n\
|PROCESS|RESPONSE|TURNAROUND|WAITING|\n\
|   ID  |  TIME  |   TIME   | TIME  |\n\
+-------+--------+----------+-------+\n");}
        printf("|   %3d |", proc[i].process_id);
        printf("  %5d |", proc[i].waiting_time);
        printf("    %5d |", proc[i].turnaround_time);
        printf(" %5d |\n", proc[i].response_time);
        if(i==proc_num-1){
            printf("+-------+--------+----------+-------+\n\n");
        }
    }
    printf("Total %d Processes\n", proc_num);
    //printf("Total Context Switch : %d\n", context_switch);
    printf("Total Runtime          : %d\n", curr_time);
    printf("Utilization            : %.3lf%%\n", utilization);
    printf("Idle Time              : %d\n", (int)idle_time);
    printf("Average Waiting Time   : %.3f\n", total_wt/proc_num);
    printf("Average Turnaround Time: %.3f\n", total_tat/proc_num);
    printf("Average Response Time  : %.3f\n\n", total_rpt/proc_num);
}

int main(){
    struct _process process[MAX_PROCESS];
    FILE *fp = fopen("result.txt", "w"); // for gantt chart

    proc_num = input_data(process);
    idle_time = total_burst_time;
    fprintf(fp, "%d\n", proc_num); // write proc_num to result.txt

    sort(process, proc_num);

    // 초기 time quantum = min(총 burst time의 평균, 처음 실행되는 프로세스의 burst time)으로 설정
    TQ = process[0].burst_time > total_burst_time/proc_num ? total_burst_time/proc_num : process[0].burst_time;
    int dupli_procn = proc_num; // while 조건에 들어갈 process 갯수

    int rq_proc = cal_rqnum_tbt(process, curr_time);

    while(dupli_procn!=0){
        //printf("total burst time : %d\n", total_burst_time);
        //printf("TQ : %d\n", TQ);
        //printf("rq_proc : %d\n", rq_proc);
        check_procstart(process);

        for(int i=0; i<rq_proc; i++){
            contswitch_count(process, i);
            if(process[i].remain_time > 0 && process[i].remain_time <= TQ){
                fprintf(fp, "%d %d ", process[i].process_id, curr_time); // write pid, start time
                proc_burst_end(process, i);
                fprintf(fp, "%d\n", curr_time); // write process return time
                dupli_procn = dupli_procn - 1;
            }
            else if(process[i].remain_time > 0 && process[i].remain_time > TQ){
                fprintf(fp, "%d %d ", process[i].process_id, curr_time); // write pid, start time
                proc_burst_remain(process, i);
                fprintf(fp, "%d\n", curr_time); // write process return time
            }

            prev_proc = process[i].process_id;
        }

        sort(process, proc_num);
        rq_proc = cal_rqnum_tbt(process, curr_time);
        sort_bt(process, rq_proc);

        if(rq_proc!=0){
            TQ = total_burst_time/rq_proc;
        }
    }

    fclose(fp);

    idle_time = curr_time - idle_time;
    utilization = (1- (idle_time/curr_time)) * 100;
    print_result(process);

    return 0;
}
