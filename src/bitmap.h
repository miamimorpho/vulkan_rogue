
typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t data[];
} BitMap;

BitMap* bitMapCreate(uint32_t, uint32_t);
void bitMapDestroy(BitMap*);
void bitMapFill(BitMap*, uint8_t);
uint8_t bitMapGetPx(BitMap*, int32_t, int32_t);
int bitMapSetPx(BitMap*, int32_t, int32_t, uint8_t);
