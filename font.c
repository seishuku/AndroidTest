#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "android_native_app_glue.h"
#include "font_6x10.h"

void point(ANativeWindow_Buffer *buffer, uint32_t x, uint32_t y, float c[3]);

void Font_PutChar(ANativeWindow_Buffer *buffer, int x, int y, char c)
{
	int i, j;

	for(j=0;j<FONT_HEIGHT;j++)
	{
		for(i=0;i<FONT_WIDTH;i++)
		{
			if(fontdata[(c*FONT_HEIGHT)+j]&(0x80>>i))
				point(buffer, x+i, y+j, (float[]){ 1.0f, 1.0f, 1.0f });
		}
	}
}

void Font_Print(ANativeWindow_Buffer *buffer, int x, int y, char *string, ...)
{
	char *ptr, text[4096]; //Big enough for full screen.
	va_list	ap;
	int sx=x;

	if(string==NULL)
		return;

	va_start(ap, string);
		vsprintf(text, string, ap);
	va_end(ap);

	for(ptr=text;*ptr!='\0';ptr++)
	{
		if(*ptr=='\n'||*ptr=='\r')
		{
			x=sx;
			y+=FONT_HEIGHT;
			continue;
		}
		if(*ptr=='\t')
		{
			x+=FONT_WIDTH*4;
			continue;
		}

		Font_PutChar(buffer, x, y, *ptr);
		x+=FONT_WIDTH;
	}
}
