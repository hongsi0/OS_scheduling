#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESS 100
#define FILE_NAME "process.txt"
#define TQ 2

int proc_num;
int curr_time = 0;
int context_switch = 0;
int prev_proc=-1;
int total_run_time = 0;
double idle_time = 0;
double utilization;


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
                total_run_time = total_run_time + atoi(pdata);
            }

            pdata = strtok(NULL, " ");
        }
        i++;
    }
    fclose(fd);

    return i;
}

void contswitch_count(struct _process *proc){
    if(curr_time > 0 && prev_proc != proc[0].process_id){
        context_switch++;
    }
}

void check_procstart(struct _process *proc){
    if(proc[0].arrival_time > curr_time){
        curr_time = curr_time + proc[0].r_at;
    }
}

void cal_response_time(struct _process *proc){
    if(proc[0].burst_time == proc[0].remain_time){
        proc[0].response_time = curr_time-proc[0].r_at;
    }
}

void proc_burst_end(struct _process *proc){
    cal_response_time(proc);
    curr_time = curr_time + proc[0].remain_time;
    proc[0].remain_time = 0;
    proc[0].turnaround_time = curr_time - proc[0].r_at;
    proc[0].waiting_time = proc[0].turnaround_time - proc[0].burst_time;
    proc[0].arrival_time = 100000000;
}

void proc_burst_remain(struct _process *proc){
    cal_response_time(proc);
    proc[0].remain_time = proc[0].remain_time - TQ;
    curr_time = curr_time + TQ;
    proc[0].arrival_time = curr_time+0.00000001;
}

void print_result(struct _process *proc){
    float total_wt = 0;
    float total_tat = 0;
    float total_rpt = 0;

    for(int i=0; i < proc_num; i++){
        total_wt = total_wt + proc[i].waiting_time;
        total_tat = total_tat + proc[i].turnaround_time;
        total_rpt = total_rpt + proc[i].response_time;
        // double utilization = (1- (idle_time/(current_time-1))) * 100;

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
    fprintf(fp, "%d\n", proc_num); // write proc_num to result.txt

    sort(process, proc_num);

    int dupli_procn = proc_num; // while 조건에 들어갈 process 갯수

    while(dupli_procn!=0){
        contswitch_count(process);
        check_procstart(process);

        if(process[0].remain_time > 0 && process[0].remain_time <= TQ){
            fprintf(fp, "%d %d ", process[0].process_id, curr_time); // write pid, start time
            proc_burst_end(process);
            fprintf(fp, "%d\n", curr_time); // write process return time

            dupli_procn = dupli_procn - 1;
        }
        else if(process[0].remain_time > 0 && process[0].remain_time > TQ){
            fprintf(fp, "%d %d ", process[0].process_id, curr_time); // write pid, start time
            proc_burst_remain(process);
            fprintf(fp, "%d\n", curr_time); // write process return time
        }

        prev_proc = process[0].process_id;
        sort(process, proc_num);
    }

    fclose(fp);

    idle_time = curr_time - total_run_time;
    utilization = (1- (idle_time/curr_time)) * 100;
    print_result(process);

    return 0;
}
