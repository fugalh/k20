#include <stdio.h>
#include <stdlib.h>
#include <jack/jack.h>

static void finish(int sig);
int *jack_process(jack_nframes_t, void*);

int main(int argc, char **argv)
{
    signal(SIGINT, finish);

    printf("-50        40        30        20   15   10  6  3  0  3  6   10   15   20+\n");
    printf(" |         |         |         |    |    |   |  |  |  |  |   |    |    |\n");
    
    finish(0);
}

static void finish(int sig)
{
    exit(0);
}

