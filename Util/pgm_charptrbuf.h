#ifndef PGM_CHARPTRBUF_H
#define PGM_CHARPTRBUF_H

typedef struct{
	char *buffer;
	unsigned int pos;
	unsigned int cap;
}CharptrBuf;

#define PGM_CHARPTRBUF_GET_DATA(charptrbuf) (charptrbuf)->buffer

CharptrBuf *CharptrBuf_init(unsigned int max);

void CharptrBuf_append(CharptrBuf *buf, char *text);

void CharptrBuf_destroy(CharptrBuf **buf);

void CharptrBuf_clear(CharptrBuf *buf);

#endif