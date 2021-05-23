#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESS 100
#define FILE_NAME "test"
#define TQ 2

int proc_num;
int start_time = 0;
int context_switch = 0, prev_proc=-1;
int tq = 2; // 타임 퀀텀
int total_run_time = 0;


struct _process{
    int process_id;
    float arrival_time;
    int r_at;
    int burst_time;
    int waiting_time;
    int response_time;
    int turnaround_time;
    int remain_time;
};

void sort(struct _process *proc, int total_proc) {
    struct _process temp;
    for(int i = 0 ; i < total_proc ; i++) {
        for(int j = i + 1 ; j < total_proc ; j++) {
            if(proc[i].arrival_time > proc[j].arrival_time) {
                temp = proc[i];
                proc[i] = proc[j];
                proc[j] = temp;
            }
        }
    }
    return;
}

void input_data(struct _process *proc){
    FILE *fd = fopen("process.txt", "r");
    char data[255];
    char *pdata;
    int i = 0;

    while(!feof(fd)){
        fgets(data, sizeof(data), fd);
        pdata = strtok(data, " ");
        for(int j = 0; j < 3; j++){
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
                total_run_time = total_run_time + atoi(pdata);
            }

            pdata = strtok(NULL, " ");
        }
        i++;
    }
    fclose(fd);

    proc_num = i;
}

void contswitch_count(struct _process *proc){
    if(start_time > 0 && prev_proc != proc[0].process_id){
        context_switch++;
    }
}

void check_procstart(struct _process *proc){
    if(proc[0].arrival_time > start_time){
        start_time = start_time + proc[0].r_at;
    }
}

void proc_burst_end(struct _process *proc){
    start_time = start_time + proc[0].remain_time;
    proc[0].remain_time = 0;
    proc[0].turnaround_time = start_time - proc[0].r_at;
    proc[0].waiting_time = proc[0].turnaround_time - proc[0].burst_time;
    proc[0].arrival_time = 100000000;
}

void proc_burst_remain(struct _process *proc){
    proc[0].remain_time = proc[0].remain_time - tq;
    start_time = start_time + tq;
    proc[0].arrival_time = start_time+0.00000001;
}

void print_gatt(int *gantt_information){

}

int main(){
    struct _process process[MAX_PROCESS];
    int gantt_info[total_run_time][2];
    FILE *fp = fopen("result.txt", "w"); // for gantt chart

    input_data(process);
    fprintf(fp, "%d\n", proc_num); // write proc_num to result.txt

    sort(process, proc_num);

    // 정렬 확인
    // for(int i=0; i < len; i++){
    //     printf("process_id: %d    ", process[i].process_id);
    //     printf("arrival_time: %d    ", process[i].arrival_time);
    //     printf("burst_time: %d\n", process[i].burst_time);
    // }

    int gantt_index = 0; // process index
    int dupli_procn = proc_num; // while 조건에 들어갈 process 갯수
    
    while(dupli_procn!=0){
        contswitch_count(process);
        check_procstart(process);

        if(process[0].remain_time > 0 && process[0].remain_time <= tq){
            fprintf(fp, "%d %d ", process[0].process_id, start_time); // write pid, start time
            proc_burst_end(process);
            fprintf(fp, "%d\n", start_time); // write process return time

            dupli_procn = dupli_procn - 1;

            // printf("case 1 %d Remain time: %d tq : %d\n", process[index].process_id, process[index].remain_time, tq);
        }
        else if(process[0].remain_time > 0 && process[0].remain_time > tq){
            fprintf(fp, "%d %d ", process[0].process_id, start_time); // write pid, start time
            proc_burst_remain(process);
            fprintf(fp, "%d\n", start_time); // write process return time

            // printf("case 2 %d Remain time: %d tq : %d\n", process[index].process_id, process[index].remain_time, tq);
        }

        gantt_index++;
        prev_proc = process[0].process_id;
        sort(process, proc_num);

        printf("%d\n", start_time);
    }
    
    printf("total context switch : %d\n", context_switch);
    printf("final total runtime : %d\n\n", start_time);

    for(int i=0; i < proc_num; i++){
        printf("process_id: %d    ", process[i].process_id);
        printf("waiting time: %d    ", process[i].waiting_time);
        printf("turnaround time: %d\n", process[i].turnaround_time);
    }

    return 0;
}