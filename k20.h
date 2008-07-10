#ifndef K20_H
#define K20_H

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <math.h>

#define min(x,y) ((x>y)?y:x)
#define max(x,y) ((x<y)?y:x)

struct meter {
    jack_port_t *port;
    jack_ringbuffer_t *ring;
    float rms;  // dBFS
    float peak; // dBFS
    float maxpeak; // dBFS
    int overs;

    float a0, a1, a2, b1, b2, x1, x2, y0, y1, y2;
};

struct context {
    jack_client_t *jack;
    struct meter m;
};

int jack_process(jack_nframes_t, void*);
float dbfs(float amplitude);
void init_meter(struct meter*, jack_nframes_t sample_rate);

#endif

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
