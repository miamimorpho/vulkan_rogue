#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "luajit.h"
#include "lua_interface.h"

#define LUA_ERROR(L, res) do {					       \
    if ((res) != LUA_OK) {					       \
      printf("fatal lua error\n");				       \
      lua_pop((L), 1); /* remove error message from stack */	       \
      lua_close((L));						       \
      return 1; /* Return from the calling function */		       \
    }								       \
  } while (0)

int loadLuaConfigTextures(Gfx gfx){

  lua_State* L = luaL_newstate();
  if(!L) {
    return 1;
  }

  int res = luaL_dofile(L, "lua/data.lua");
  LUA_ERROR(L, res);
  
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
