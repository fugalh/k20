#include "k20.h"

float dbfs(float amplitude) 
{
    return 20*log10(max(amplitude, 0));
}

void init_meter(struct meter *m, jack_nframes_t Fs)
{
    // See http://www.earlevel.com/Digital%20Audio/Bilinear.html
    float Fc = 2.224, Q = 0.6053;
    float K = tan(M_PI * Fc/Fs);
    float K2 = K*K;
    float KQ = K/Q;
    m->a0 = K2/(K2 + KQ + 1.0);
    m->a1 = 2*m->a0;
    m->a2 = m->a0;
    m->b1 = 2*(K2 - 1.0)/(K2 + KQ + 1.0);
    m->b2 = (K2 - KQ + 1.0)/(K2 + KQ + 1.0);
    m->x1 = m->x2 = m->y0 = m->y1 = m->y2 = 0;
    m->rms = m->peak = m->maxpeak = dbfs(0);
}

void rms(struct meter *m, float x0)
{
    /*
                     M
                    SUM a(k+1) z^(-k)
                    k=0
        H(z) = ----------------------
                     N
                1 + SUM b(k+1) z^(-k)
                    k=1
       so, 
        
        y(n) = sum(k=0..M; a(k+1) x(n-k)) - sum(k=1..N; b(k+1) y(n-k))

    */

    x0 = x0*x0;

    // calculate
    m->y0 = m->a0 * x0 + m->a1 * m->x1 + m->a2 * m->x2 - 
                        (m->b1 * m->y1 + m->b2 * m->y2);
    // avoid a sqrt by dividing log by 2.
    // adjust up for AES-17 (not sure why it's off by about 1.4 instead of 3)
    m->rms = 10*log10(m->y0) + 1.4;

    // update
    m->x2 = m->x1;
    m->x1 = x0;
    m->y2 = m->y1;
    m->y1 = m->y0;
}

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
    for (i=0; i<nframes; i++)
        rms(m,buf[i]);

    // overs
    int c = 0;
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
