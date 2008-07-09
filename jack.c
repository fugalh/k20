#include "k20.h"

// XXX not sure about the ballistics. Right now, just linear falloff which
// doesn't seem quite right.
int jack_process(jack_nframes_t nframes, void *arg)
{
    struct context *ctx = (struct context*)arg;

    jack_nframes_t sr = jack_get_sample_rate(ctx->jack);
    float s = (float)nframes/sr;
    struct meter *m = &(ctx->m);
    float *buf = (float*)jack_port_get_buffer(m->port, nframes);

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
    m->peak -= s * 26/3;
    if (peak > m->peak)
        m->peak = peak;

    // max peak
    if (m->peak > m->maxpeak)
        m->maxpeak = m->peak;

    // RMS
    ringbuffer_read_advance(m->ring, nframes);
    ringbuffer_write(m->ring, buf, nframes);
    int c = (int)(sr * 0.3);
    float rmsbuf[c];
    ringbuffer_peek(m->ring, rmsbuf, c);
    float sum = 0;
    for (i=0; i<c; i++)
        sum += rmsbuf[i]*rmsbuf[i];
    m->rms = dbfs(sqrt(sum/c)); 

    // overs
    c = 0;
    for (i=0; i<nframes; i++)
    {
        float x = fabsf(buf[i]);
        if (x >= 1.0)
        {
            if (++c == 3)
                m->overs++;
        } else
            c = 0;
    }

    return 0;
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
