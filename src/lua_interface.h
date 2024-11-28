#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "luajit.h"
#include "vkterm/vkterm.h"
#include "world.h"

int loadLuaConfigTextures(Gfx gfx);
lua_State* loadLuaConfigControls(void);
int objectLuaRunScript(lua_State* L, GameObject* object_ptr);
