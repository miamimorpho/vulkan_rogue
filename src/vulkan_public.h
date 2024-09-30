
int inputInit(void);
int gfxConstInit(void);
int gfxGlobalInit(void);

int gfxTextureLoad(const char*);

int gfxDrawStart(void);
int gfxBlitImage(int);
int gfxDrawChar(char, int, int, int, int);
int gfxDrawString(const char*, int, int, int);
int gfxDrawEnd(void);
int gfxClose(void);
