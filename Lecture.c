#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>

static int compare (const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}

double getTime(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (1.0e-6*t.tv_usec + t.tv_sec);
}

// A = 65 = 0100 0001
// T = 84 = 0101 0100
// C = 67 = 0100 0011
// G = 71 = 0100 0111

int main (int argc, char *argv[]){

    FILE *fp;
    FILE *fw;

    char * line_void = NULL;
    volatile int lines = 0;
    size_t len = 0;
    volatile double t1;
    volatile double t2;

    //#######################################################################################################
    // Get coverage
    printf("Analysing coverage ...\n");
    t1 = getTime();
    fp = fopen("reads.fasta", "r");
    getline(&line_void, &len, fp);
    int coverage = getline(&line_void, &len, fp);
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);
    fclose(fp);

    //#######################################################################################################
    // Get lines number
    printf("Analysing line number ...\n");
    t1 = getTime();
    fp = fopen("reads.fasta", "r");
    struct statfs fsInfo = {0};
    int fd = fileno(fp); 
    long optimalSize;

    struct stat buf;
    fstat(fd, &buf);
    long file_size = buf.st_size;

    if (fstatfs(fd, &fsInfo) == -1) {
        optimalSize = 4 * 1024 * 1024;
    } else {
        optimalSize = fsInfo.f_bsize;
    }

    volatile long size_counter = 0;
    char *BUFFER_SECONDARY = malloc(optimalSize);

    while(size_counter < file_size){
        fread(BUFFER_SECONDARY, optimalSize, 1,fp);
        size_counter = size_counter + optimalSize;
        for(int i = 0; i < optimalSize ; i++){
            if(BUFFER_SECONDARY[i] == '\n'){
                lines++;
            }
        }
    }


    free(BUFFER_SECONDARY);
    fclose(fp);
    t2 = getTime();
    //printf ("%d\n",lines);
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Regularizing values and creating BUFFER
    printf("Creating primary Buffer ...\n");
    t1 = getTime();
    int MAX_SIZE = lines/2;
    

    char **BUFFER_ONE = malloc(MAX_SIZE * sizeof(char*));
    for (int i = 0; i < MAX_SIZE; i++){
        BUFFER_ONE[i] = malloc((coverage) * sizeof(char));
    }
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Buffering file content
    printf("Buffering file ...\n");
    t1 = getTime();
    fp = fopen("reads.fasta", "r");
    len = 0;
    volatile unsigned int counter = 0;
    getline(&line_void, &len, fp);
    while(getline(&BUFFER_ONE[counter], &len, fp) != -1){
        getline(&line_void, &len, fp);
        counter++;
    }
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Sorting the sequences by alphabetical order
    printf("Sorting buffer ...\n");
    t1 = getTime();
    qsort (BUFFER_ONE, MAX_SIZE, sizeof(char *), compare);

    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    fw = fopen("reads_sorted.fasta","w");
    for(int i = 0; i < MAX_SIZE; i++){
        fprintf (fw, "%s",BUFFER_ONE[i]);
    }
    fclose(fw);

    //#######################################################################################################
    // Deleting string without duplicate and saving strings with duplicate(s)
    printf("Processing ...\n");
    t1 = getTime();

    char **BUFFER_TWO = malloc(MAX_SIZE * sizeof(char*));
    //char *memory = malloc(coverage * sizeof(char));
    volatile unsigned int scanner_cursor = 1;
    volatile unsigned int buffer_two_cursor = 0;
    volatile int zero_is_a_precedent_duplicate = 0;
    volatile int str_cmp_res = 0;

    while (scanner_cursor < MAX_SIZE)
    {
        //We save the comparaison between the strings of the two cursors
        str_cmp_res = strcmp(BUFFER_ONE[scanner_cursor - 1], BUFFER_ONE[scanner_cursor]);

        //If we know that the first cursor don't containt a previous duplicate
        if(zero_is_a_precedent_duplicate == 0){

            //If the strings are the sames, then we save one of them into the second buffer, and we activate the previous duplicate alert
            if(str_cmp_res == 0){
                BUFFER_TWO[buffer_two_cursor] = BUFFER_ONE[scanner_cursor - 1];
                buffer_two_cursor++;
                zero_is_a_precedent_duplicate = 1;
            }
        
        //If we know that the first cursor containt a previous duplicate
        }else{

            //If the strings in the two cursor are differents we cancel the previous duplicate alert
            if(str_cmp_res != 0){
                zero_is_a_precedent_duplicate = 0;
                
            }
        }
        scanner_cursor++;
    }

    free(BUFFER_ONE);
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Writing file
    printf("Writing file ...\n");
    t1 = getTime();
    fw = fopen("reads_processed.fasta","w");
    for(int i = 0; i < buffer_two_cursor; i++){
        fprintf (fw, "%s",BUFFER_TWO[i]);
    }
    fclose(fw);
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    //#######################################################################################################
    // Closing everything
    free(BUFFER_TWO);
    

}