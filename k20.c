#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jack/jack.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>


struct context {
    sem_t *sem;
    jack_client_t *jack;
    jack_port_t *in;
    float peak; // dBFS
    float rms;  // dBFS
    float maxpeak; // dBFS
    int overs;
};

#define min(x,y) (((x)>(y))?(y):(x))
#define max(x,y) (((x)<(y))?(y):(x))
int jack_process(jack_nframes_t, void*);
float dbfs(float amplitude) { return 20*log(max(amplitude, 0) / 1.0); }
int scale(float dbfs);

int main(int argc, char **argv)
{
    struct context ctx;
    ctx.peak = ctx.rms = ctx.maxpeak = dbfs(0);
    ctx.overs = 0;
    printf("-70   60   50   40   30        20   15   10  6  3  0  3  6   10   15   20+\n");
    printf(" |    |    |    |    |         |    |    |   |  |  |  |  |   |    |    |\n");
    
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
        char meter[71];

        memset(meter, ' ', 70);
        meter[70] = 0;
        int p = scale(ctx.rms);
        if (p >= 0)
            memset(meter, '#', p);
        p = scale(ctx.peak)-1;
        if (p >= 0)
            meter[p] = '#';
        p = scale(ctx.maxpeak)-1;
        if (p >= 0)
            meter[p] = '#';

        printf(" \e[K\e[32m%.50s\e[33m%.5s\e[31m%.15s\e[0m", meter, meter+50, meter+55);
        if (ctx.overs > 0)
            printf("    \e[41;37m %d \e[0m", ctx.overs);
#ifdef DEBUG
        printf(" %d %d", (int)ctx.peak, scale(ctx.peak));
#endif
        printf("\r");

        fflush(stdout);
    }

    jack_deactivate(ctx.jack);
    jack_client_close(ctx.jack);
}

int scale(float dbfs)
{
    int x;
    if (dbfs < -50)
        x = (90+dbfs)/2;
    else
        x = 70+dbfs;

    return max(0, min(x, 70));
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
    // XXX instantaneous rise isn't right, probably
    float sum = 0;
    for (i=0; i<nframes; i++)
        sum += buf[i]*buf[i];
    float rms = dbfs(sqrt(sum/nframes)); 
    ctx->rms -= s * 70/0.3; // 300ms fall time
    if (rms > ctx->rms)
        ctx->rms = rms;

    // overs
    int c = 0;
    for (i=0; i<nframes; i++)
    {
        float x = fabsf(buf[i]);
        if (x >= 1.0)
        {
            if (++c == 3)
                ctx->overs++;
        } else
            c = 0;
    }

    sem_post(ctx->sem);
}

/*
    Copyright (C) 2008  Hans Fugal

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
