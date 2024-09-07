typedef struct{
  int x;
  int y;
} pos_t;

typedef struct{
  pos_t pos;
  char ch;
} entity_t;

typedef struct{
  entity_t* tiles;
  int width;
  int size;
  entity_t* props;
  entity_t* actors;
  int actors_count;
} gameWorld_t;

int mapPutTile(gameWorld_t*, entity_t, int, int);
int worldInit(gameWorld_t*, int);
int entityMove(entity_t*, int, int);
int entityAdd(gameWorld_t*, entity_t);
