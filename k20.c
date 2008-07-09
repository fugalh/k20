#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <sys/select.h>

#include "ringbuffer.h"
#include "options.h"
#include "k20.h"

int scale(float dbfs);
float dbfs(float amplitude) { return 20*log(max(amplitude, 0) / 1.0); }

int main(int argc, char **argv)
{
    struct options opts = {0, "k20"};
    parse_options(&argc, argv, &opts);

    struct context ctx;

    ctx.m.peak = ctx.m.rms = ctx.m.maxpeak = dbfs(0);
    ctx.m.overs = 0;
    printf("-70   60   50   40   30        20   15   10  6  3  0  3  6   10   15   20+\n");
    printf(" |    |    |    |    |         |    |    |   |  |  |  |  |   |    |    |\n");
    
    // JACK initialization
    ctx.jack = jack_client_open(opts.n, 0, 0);
    if (!ctx.jack)
    {
        fprintf(stderr, "Failed to create JACK client.\n");
        exit(1);
    }

    ctx.m.port = jack_port_register(ctx.jack, "in", JACK_DEFAULT_AUDIO_TYPE, 
                                    JackPortIsInput|JackPortIsTerminal, 0);
    if (!ctx.m.port)
    {
        fprintf(stderr, "Failed to create input port.\n");
        exit(1);
    }
    
    if (jack_set_process_callback(ctx.jack, jack_process, &ctx) != 0)
    {
        fprintf(stderr, "Failed to set process callback.\n");
        exit(1);
    }

    jack_nframes_t sr = jack_get_sample_rate(ctx.jack);
    ctx.m.ring = ringbuffer_create(sr * 0.3);
    if (ctx.m.ring == 0)
    {
        fprintf(stderr, "Failed to create ringbuffer.\n");
    }

    jack_activate(ctx.jack);

    while (1)
    {
        // reset?
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        struct timeval tv = {0,40000};
        if (select(fileno(stdin)+1, &readfds, 0, 0, &tv))
        {
            char buf[1024];
            fgets(buf, 1024, stdin);
            ctx.m.overs = 0;
            ctx.m.maxpeak = ctx.m.peak = dbfs(0);
        }

        char meter[72];

        memset(meter, ' ', 71);
        meter[71] = 0;
        int p = scale(ctx.m.rms);
        if (p >= 0)
            memset(meter, '#', p);
        p = scale(ctx.m.peak)-1;
        if (p >= 0)
            meter[p] = '#';
        p = scale(ctx.m.maxpeak)-1;
        if (p >= 0)
            meter[p] = '#';

        printf(" \e[K\e[32m%.50s\e[33m%.5s\e[31m%.16s\e[0m", meter, meter+50, meter+55);
        if (ctx.m.overs > 0)
            printf("    \e[41;37m %d \e[0m", ctx.m.overs);
        if (opts.d)
            printf(" %d %d %d", (int)ctx.m.rms, (int)ctx.m.peak, (int)ctx.m.maxpeak);
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
        x = 71+dbfs;

    return max(0, min(x, 71));
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
