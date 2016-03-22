#include <string.h>

#include "pgm_charptrbuf.h"
#include "pgm_malloc.h"

CharptrBuf *CharptrBuf_init(unsigned int max){
	CharptrBuf *buf = (CharptrBuf*) pgm_malloc (sizeof(CharptrBuf));
	buf->buffer = (char*) pgm_malloc (sizeof(char)*max);
	buf->cap = max;
	buf->pos = 0;
	return buf;
}

void CharptrBuf_append(CharptrBuf *buf, char *text){
	unsigned int len_text = strlen(text);
	if(len_text + buf->pos > buf->cap){		
		buf->cap *= 2;
		buf->buffer = (char*) pgm_realloc (buf->buffer, sizeof(char)*buf->cap);
	}
	strcpy(&buf->buffer[buf->pos],text);
	buf->pos += len_text;
}

void CharptrBuf_destroy(CharptrBuf **buf){
	pgm_free((*buf)->buffer);
	pgm_free(*buf);
	*buf = NULL;
}

void CharptrBuf_clear(CharptrBuf *buf){
	memset(buf->buffer,0,sizeof(char)*buf->cap);
	buf->pos = 0;
}
