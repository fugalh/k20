#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jack/jack.h>
#include <semaphore.h>
#include <math.h>

struct context {
    sem_t *sem;
    jack_client_t *jack;
    jack_port_t *in;
    float peak; // dBFS
    float rms;  // dBFS
    float maxpeak; // dBFS
};

int jack_process(jack_nframes_t, void*);
float dbfs(float amplitude) { return 20*log(amplitude/1.0); }

int main(int argc, char **argv)
{
    struct context ctx;
    ctx.maxpeak = dbfs(0);
    printf("-50        40        30        20   15   10  6  3  0  3  6   10   15   20+\n");
    
    // JACK initialization
    ctx.jack = jack_client_open("k20", 0, 0);
    if (!ctx.jack)
    {
        fprintf(stderr, "Failed to create JACK client.\n");
        exit(1);
    }

    ctx.sem = sem_open(jack_get_client_name(ctx.jack), O_CREAT, 0600, 0);
    if (ctx.sem == SEM_FAILED)
    {
        perror("opening semaphore");
        exit(1);
    }

    ctx.in = jack_port_register(ctx.jack, "in", JACK_DEFAULT_AUDIO_TYPE, 
                                JackPortIsInput|JackPortIsTerminal, 0);
    if (!ctx.in)
    {
        fprintf(stderr, "Failed to create input port.\n");
        exit(1);
    }
    
    if (jack_set_process_callback(ctx.jack, jack_process, &ctx) != 0)
    {
        fprintf(stderr, "Failed to set process callback.\n");
        exit(1);
    }

    jack_activate(ctx.jack);

    while (sem_wait(ctx.sem) != -1)
    {
        printf("\r%15f %15f %15f", ctx.peak, ctx.rms, ctx.maxpeak);
        fflush(stdout);
    }

    jack_deactivate(ctx.jack);
    jack_client_close(ctx.jack);
}

int jack_process(jack_nframes_t nframes, void *arg)
{
    struct context *ctx = (struct context*)arg;

    jack_nframes_t sr = jack_get_sample_rate(ctx->jack);
    float *buf = (float*)jack_port_get_buffer(ctx->in, nframes);

    // peak
    float peak = 0;
    int i;
    for (i=0; i<nframes; i++)
        if (buf[i] > peak)
            peak = buf[i];
    ctx->peak = dbfs(peak/1.0); // dBFS

    // max peak
    if (ctx->peak > ctx->maxpeak)
        ctx->maxpeak = ctx->peak;

    // RMS
    // XXX the buffer is probably not the appropriate window 
    float sum = 0;
    for (i=0; i<nframes; i++)
        sum += buf[i]*buf[i];
    ctx->rms = dbfs(sqrt(sum/nframes));

    sem_post(ctx->sem);
}
