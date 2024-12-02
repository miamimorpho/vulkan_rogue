#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "luajit.h"
#include "vkterm/vkterm.h"
#include "world.h"

int luaLoadTextures(Gfx gfx);
lua_State* luaStateInit(Gfx gfx, const char*);
int luaControls(lua_State* L, GameObject* object_ptr);
