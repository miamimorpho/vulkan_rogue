/*
#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "luajit.h"

int loadLuaConfigs(Gfx gfx){

  lua_State* L = luaL_newstate();
  if(!L) {
    return 1;
  }

  int res = luaL_dofile(L, "lua/data.lua");
  if(res != LUA_OK){
    printf("error on lua\n");
  }

  lua_getglobal(L, "textures");
  int texture_c = lua_objlen(L, -1);
  printf("number of textures: %d\n", num_textures);

  for(int i = 1; i <= texture_c; i++){
    lua_rawgeti(L, -1, i);
    if(lua_isstring(L, -1)){
        gfxTextureLoad(gfx, "textures/color.png");
    }
  }
  
  lua_close(L);
  
  return 0;
}
*/
