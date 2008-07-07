#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jack/jack.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>


struct context {
    sem_t *sem;
    jack_client_t *jack;
    jack_port_t *in;
    float peak; // dBFS
    float rms;  // dBFS
    float maxpeak; // dBFS
};

#define min(x,y) ((x>y)?y:x)
#define max(x,y) ((x<y)?y:x)
int jack_process(jack_nframes_t, void*);
float dbfs(float amplitude) { return 20*log(max(amplitude, 0) / 1.0); }

int main(int argc, char **argv)
{
    struct context ctx;
    ctx.peak = ctx.rms = ctx.maxpeak = dbfs(0);
    printf("-50        40        30        20   15   10  6  3  0  3  6   10   15   20+\n");
    printf(" |         |         |         |    |    |   |  |  |  |  |   |    |    |\n");
    
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
        char meter[72];

        memset(meter, ' ', 71);
        meter[71] = 0;
        memset(meter, '#', min(max(0, 71+(int)ctx.rms), 71));
        meter[min(max(0, 71+(int)ctx.peak), 71)] = '#';
        meter[min(max(0, 71+(int)ctx.maxpeak), 71)] = '#';

        printf("\e[K\e[32m%.51s\e[33m%.5s\e[31m%s\e[0m %d %d %d\r", meter, meter+51, meter+56, (int)ctx.rms, (int)ctx.peak, (int)ctx.maxpeak);
        fflush(stdout);
    }

    jack_deactivate(ctx.jack);
    jack_client_close(ctx.jack);
}

// XXX not sure about the ballistics. Right now, just linear falloff which
// doesn't seem quite right.
int jack_process(jack_nframes_t nframes, void *arg)
{
    struct context *ctx = (struct context*)arg;

    jack_nframes_t sr = jack_get_sample_rate(ctx->jack);
    float s = (float)nframes/sr;
    float *buf = (float*)jack_port_get_buffer(ctx->in, nframes);

    // peak
    float peak = 0;
    int i;
    for (i=0; i<nframes; i++)
    {
        float x = fabsf(buf[i]);
        if (x > peak)
            peak = x;
    }
    peak = dbfs(peak);
    ctx->peak -= s * 26/3;
    if (peak > ctx->peak)
        ctx->peak = peak;

    // max peak
    if (ctx->peak > ctx->maxpeak)
        ctx->maxpeak = ctx->peak;

    // RMS
    // XXX the buffer is probably not the appropriate window ?
    float sum = 0;
    for (i=0; i<nframes; i++)
        sum += buf[i]*buf[i];
    float rms = dbfs(sqrt(sum/nframes)); 
    ctx->rms -= s * 70/0.3; // 300ms fall time
    if (rms > ctx->rms)
        ctx->rms = rms;

    // overs
    //
    sem_post(ctx->sem);
}
