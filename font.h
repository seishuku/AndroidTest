#ifndef __font_h_
#define __font_h_
#ifdef __cplusplus
extern "C" {
#endif

void Font_Print(ANativeWindow_Buffer *buffer, int x, int y, char *string, ...);

#ifdef __cplusplus
}
#endif
#endif
