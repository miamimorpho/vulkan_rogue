#include <stdio.h>

#include "lua_interface.h"

#define LUA_ERROR(L, res) do {                                          \
        if ((res) != LUA_OK) {                                          \
            fprintf(stderr, "Load error: %s\n", lua_tostring(L, -1));   \
            lua_pop((L), 1); /* remove error message from stack */      \
        }                                                               \
    } while (0)



int luaLoadTextures(Gfx gfx){

  lua_State* L = luaL_newstate();
  if(!L) {
    return 1;
  }

  int res = luaL_dofile(L, "lua/data.lua");
  if(res != LUA_OK){
      LUA_ERROR(L, res);
      return 1;
  }
  
  lua_getglobal(L, "textures");
  if(!lua_istable(L, -1)) return 1;
  int texture_c = lua_objlen(L, -1);
  for(int i = 1; i <= texture_c; i++){
    lua_pushinteger(L, i);
    lua_gettable(L, -2);
    const char *filename = lua_tostring(L, -1);
    gfxTextureLoad(gfx, filename);
    lua_pop(L, 1);
  }

  lua_pop(L, 1);
  lua_close(L);
  
  return 0;
}

lua_State* luaStateInit(Gfx gfx, const char* script_name){
    lua_State* L = luaL_newstate();
    if(L == NULL) return NULL;

    luaL_openlibs(L);

    lua_pushlightuserdata(L, gfx);
    lua_setglobal(L, "GFX_IMPL");

    int res = luaL_dofile(L, script_name);
    if(res != LUA_OK){
        LUA_ERROR(L, res);
        return NULL;
    }

    return L;
}


int luaControls(lua_State* L, GameObject* object_ptr){

  lua_getglobal(L, "handleInput");
  lua_pushlightuserdata(L, object_ptr);

  int res = lua_pcall(L, 1, 0, 0);

  if(res != LUA_OK){
      LUA_ERROR(L, res);
      return 1;
  }
  
  return 0;
}
