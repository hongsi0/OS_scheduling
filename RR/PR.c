#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESS 100
#define FILE_NAME "test2.txt"

int proc_num;
int start_time = 0;
int context_switch = 1;
int prev_proc=-1;
int total_run_time = 0;
int total_burst_time = 0;
int TQ = 0;


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

void sort(struct _process *proc, int total_proc, int finished_proc) {
    struct _process temp;
    for(int i = finished_proc ; i < total_proc ; i++) {
        for(int j = i + 1 ; j < total_proc ; j++) {
            if(proc[i].arrival_time > proc[j].arrival_time) {
                temp = proc[i];
                proc[i] = proc[j];
                proc[j] = temp;
            }
        }
    }
}

void sort_bt(struct _process *proc, int rq_proc, int finished_procnum) {
    struct _process temp;
    for(int i = 0 ; i < rq_proc; i++) {
        for(int j = i + 1 ; j < rq_proc; j++) {
            if(proc[i].remain_time > proc[j].remain_time) {
                temp = proc[i];
                proc[i] = proc[j];
                proc[j] = temp;
            }
        }
    }
}

int input_data(struct _process *proc){
    FILE *fd = fopen(FILE_NAME, "r");
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
                total_burst_time = total_burst_time + atoi(pdata);
                total_run_time = total_run_time + atoi(pdata);
            }
            pdata = strtok(NULL, " ");
        }
        i++;
    }
    fclose(fd);

    return i;
}

void contswitch_count(struct _process *proc, int running_proc){
    if(start_time > 0 && prev_proc != proc[running_proc].process_id){
        context_switch++;
    }
}

void check_procstart(struct _process *proc){
    if(proc[0].arrival_time > start_time){
        start_time = start_time + proc[0].r_at;
    }
}

void cal_response_time(struct _process *proc, int running_proc){
    if(proc[running_proc].burst_time == proc[running_proc].remain_time){
        proc[running_proc].response_time = start_time-proc[running_proc].r_at;
    }
}

void proc_burst_end(struct _process *proc, int running_proc){
    cal_response_time(proc, running_proc);
    start_time = start_time + proc[running_proc].remain_time;
    proc[running_proc].remain_time = 0;
    proc[running_proc].turnaround_time = start_time - proc[running_proc].r_at;
    proc[running_proc].waiting_time = proc[running_proc].turnaround_time - proc[running_proc].burst_time;
    proc[running_proc].arrival_time = 100000000;
}

void proc_burst_remain(struct _process *proc, int running_proc){
    cal_response_time(proc, running_proc);
    proc[running_proc].remain_time = proc[running_proc].remain_time - TQ;
    start_time = start_time + TQ;
    proc[running_proc].arrival_time = start_time+0.00000001;
}

int cal_rqnum_tbt(struct _process *proc, int curr_time, int running_proc){
    int proc_count = 0;
    total_burst_time = 0;
    
    for(int i=running_proc; i<proc_num; i++){
        if(proc[i].arrival_time > curr_time+0.1){
            break;
        }

        total_burst_time = total_burst_time + proc[i].remain_time;
        proc_count++;
    }
    return proc_count;
}

void print_result(struct _process *proc){
    int total_wt = 0;
    int total_tat = 0;
    int total_rpt = 0;

    for(int i=0; i < proc_num; i++){
        total_wt = total_wt + proc[i].waiting_time;
        total_tat = total_tat + proc[i].turnaround_time;
        total_rpt = total_rpt + proc[i].response_time;

        if(i==0){
            printf("\n\n-------------------------------------------------------------------------------\n");
        }
        printf("process_id: %d  |  ", proc[i].process_id);
        printf("waiting time: %d  |  ", proc[i].waiting_time);
        printf("turnaround time: %d  |  ", proc[i].turnaround_time);
        printf("response time: %d\n", proc[i].response_time);
        if(i==proc_num-1){
            printf("-------------------------------------------------------------------------------\n");
        }
    }
    printf("Total Context Switch : %d\n", context_switch);
    printf("Total Runtime : %d\n", start_time);
    printf("Average Waiting Time : %d\n", total_wt/proc_num);
    printf("Average Turnaround Time : %d\n", total_tat/proc_num);
    printf("Average Response Time : %d\n\n", total_rpt/proc_num);
}

int main(){
    struct _process process[MAX_PROCESS];
    FILE *fp = fopen("result.txt", "w"); // for gantt chart

    proc_num = input_data(process);
    fprintf(fp, "%d\n", proc_num); // write proc_num to result.txt

    sort(process, proc_num, 0);

    TQ = process[0].burst_time > total_burst_time/proc_num ? total_burst_time/proc_num : process[0].burst_time;
    int dupli_procn = proc_num; // while 조건에 들어갈 process 갯수

    int rq_proc = cal_rqnum_tbt(process, start_time, 0);
    
    while(dupli_procn!=0){
        printf("total burst time : %d\n", total_burst_time);
        printf("TQ : %d\n", TQ);
        printf("rq_proc : %d\n", rq_proc);
        check_procstart(process);

        for(int i=0; i<rq_proc; i++){
            contswitch_count(process, i);
            if(process[i].remain_time > 0 && process[i].remain_time <= TQ){
                fprintf(fp, "%d %d ", process[i].process_id, start_time); // write pid, start time
                proc_burst_end(process, i);
                fprintf(fp, "%d\n", start_time); // write process return time
                dupli_procn = dupli_procn - 1;
            }
            else if(process[i].remain_time > 0 && process[i].remain_time > TQ){
                fprintf(fp, "%d %d ", process[i].process_id, start_time); // write pid, start time
                proc_burst_remain(process, i);
                fprintf(fp, "%d\n", start_time); // write process return time
            }

            prev_proc = process[i].process_id;
        }

        sort(process, proc_num, 0);
        rq_proc = cal_rqnum_tbt(process, start_time, 0);
        sort_bt(process, rq_proc, 0);

        if(rq_proc!=0){
            TQ = total_burst_time/rq_proc;
        }
    }

    fclose(fp);

    print_result(process);

    return 0;
}