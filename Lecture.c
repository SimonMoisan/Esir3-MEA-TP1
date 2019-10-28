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

    // Get coverage
    printf("Analysing coverage ...\n");
    t1 = getTime();
    fp = fopen("reads.fasta", "r");
    getline(&line_void, &len, fp);
    int coverage = getline(&line_void, &len, fp);
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);
    fclose(fp);

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

    // Sorting the sequences by alphabetical order
    printf("Sorting buffer ...\n");
    t1 = getTime();t1 = getTime();
    qsort (BUFFER_ONE, MAX_SIZE, sizeof(char *), compare);

    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    // Deleting duplicate
    char **BUFFER_TWO = malloc(MAX_SIZE * sizeof(char*));

    free(BUFFER_TWO);

    // Writing file
    printf("Writing file ...\n");
    t1 = getTime();
    fw = fopen("reads_sorted.fasta","w");
    for(int i = 0; i < MAX_SIZE; i++){
        fprintf (fw, "%s",BUFFER_ONE[i]);
    }
    fclose(fw);
    t2 = getTime();
    printf (" - time: %1.2lf sec\n",t2-t1);

    // Closing everything
    free(BUFFER_ONE);
    

}