#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jack/jack.h>

int jack_process(jack_nframes_t, void*);

int main(int argc, char **argv)
{
    printf("-50        40        30        20   15   10  6  3  0  3  6   10   15   20+\n");
    printf(" |         |         |         |    |    |   |  |  |  |  |   |    |    |\n");
    
    // JACK initialization
    jack_client_t *j = jack_client_open("k20", 0, 0);
    if (!j)
    {
        fprintf(stderr, "Failed to create JACK client.\n");
        exit(1);
    }

    jack_port_t *p = jack_port_register(j, "in", JACK_DEFAULT_AUDIO_TYPE, 
                                        JackPortIsInput|JackPortIsTerminal, 0);
    if (!p)
    {
        fprintf(stderr, "Failed to create input port.\n");
        exit(1);
    }
    
    if (jack_set_process_callback(j, jack_process, 0) != 0)
    {
        fprintf(stderr, "Failed to set process callback.\n");
        exit(1);
    }

    // tell jack go
    jack_activate(j);

    sleep(30);
    jack_deactivate(j);
    jack_client_close(j);
}

int jack_process(jack_nframes_t nframes, void *arg)
{
}
