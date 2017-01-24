#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    pid_t pids[3];
    int i;
    int n = 3;

    /* Start child processes */
    for (i = 0; i < n; ++i) {
        if ((pids[i] = fork()) < 0) {
            perror("fork");
            abort();
        }
        else if (pids[i] == 0) {
            if (i == 0){
                /* run pdm test */
                printf("run pdm test\n");
                execl("./test_pdm","test_pdm",NULL);
            }
            else if (i == 1){
                /* run camera test */
                printf("run camera test\n");
            }
            else if (i == 2){
                /* run photodiode test */
                printf("run photodiode test\n");
            }
        exit(0);
        }
    }
    

    /* Wait for child processes to exit */
    int status;
    pid_t pid;
    while (n > 0) {
      pid = wait(&status);
      printf("Process with PID %ld exited with status 0x%x.\n", (long)pid, status);
      --n;  // TODO(pts): Remove pid from the pids array.
    }

    return 0;
}