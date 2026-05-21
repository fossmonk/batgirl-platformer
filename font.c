#include <stdio.h>
#include "tigr/tigr.h"

void font_load_ascii(char *fontpath, TigrFont **font) {
    Tigr *fontimage = tigrLoadImage(fontpath);
    if(fontimage) {
        *font = tigrLoadFont(fontimage, TCP_ASCII);
    } else {
        *font = tfont;
    }
}