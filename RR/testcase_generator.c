#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define _CRT_SECURE_NO_WARNINGS

int main(int argc, char* argv[]){
    int n;

    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        scanf("%d",&n);
    }

    FILE *fp = fopen("process.txt", "w");

    srand(time(NULL));
    if(n < 50) {
        for(int i=0; i<n-1; i++){
            fprintf(fp, "%d %d\n", rand()%51, (rand()%13)+1);
        }
        fprintf(fp, "%d %d", rand()%51, (rand()%13)+1);
    } else {
        for(int i=0; i<n-1; i++){
            fprintf(fp, "%d %d\n", rand()%101, (rand()%16)+1);
        }
        fprintf(fp, "%d %d", rand()%101, (rand()%16)+1);
    }

    fclose(fp);    // 파일 포인터 닫기

    return 0;
}
