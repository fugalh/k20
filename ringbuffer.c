#include "ringbuffer.h"
#include <string.h>

jack_ringbuffer_t *ringbuffer_create(size_t sz)
{
    jack_ringbuffer_t *rb = jack_ringbuffer_create(sz * sizeof(float));
    if (rb)
    {
        float buf[sz];
        int i;
        for (i=0; i<sz; i++)
            buf[i] = 0;
        ringbuffer_write(rb, buf, sz);
    }

    return rb;
}

size_t ringbuffer_peek(jack_ringbuffer_t *rb, float *dest, size_t cnt)
{
    return jack_ringbuffer_peek(rb, (char*)dest, cnt * sizeof(float))/sizeof(float);
}

void ringbuffer_read_advance(jack_ringbuffer_t *rb, size_t cnt)
{
    jack_ringbuffer_read_advance(rb, cnt * sizeof(float));
}

size_t ringbuffer_write(jack_ringbuffer_t *rb, float *src, size_t cnt)
{
    return jack_ringbuffer_write(rb, (char*)src, cnt * sizeof(float))/sizeof(float);
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
