-- input_config.lua

local ffi = require("ffi")

ffi.cdef[[
    struct GameObject;
    typedef struct GameObject GameObject;

    int lMouseX(void);
    int lMouseY(void);
    int lGetUnicode(void);
    int lMoveMobile(GameObject* object_ptr, int dx, int dy);
]]

-- Define input mappings

local UNICODE_PUA_START = 0xE000  -- Start of Private Use Area
local UNICODE_PUA_END = 0xF8FF   -- End of Private Use Area

-- Mouse button mappings in PUA
local MOUSE_BUTTON_CODES = {
    [UNICODE_PUA_START + 1] = 'left_click',
    [UNICODE_PUA_START + 2] = 'right_click',
    [UNICODE_PUA_START + 3] = 'middle_click',
    [UNICODE_PUA_START + 4] = 'scroll_up',
    [UNICODE_PUA_START + 5] = 'scroll_down'
}

local CONTROLS = {
      ['w'] = function() ffi.C.lMoveMobile(game_object, 0, -1) end,    -- Move up
      ['s'] = function() ffi.C.lMoveMobile(game_object, 0, 1) end,     -- Move down
      ['a'] = function() ffi.C.lMoveMobile(game_object, -1, 0) end,    -- Move left
      ['d'] = function() ffi.C.lMoveMobile(game_object, 1, 0) end,     -- Move right
}

-- Function to handle input
function handleInput()

    if not CURRENT_GAME_OBJECT then
        error("No current game object set")
    end
    game_object = ffi.cast("GameObject*", CURRENT_GAME_OBJECT)
    
    local mouse_x = ffi.C.lMouseX()
    local mouse_y = ffi.C.lMouseY()

    local unicode_int = ffi.C.lGetUnicode()
    if unicode_int >= UNICODE_PUA_START then
       local unicode_pua = MOUSE_BUTTON_CODES[unicode_int]
       if unicode_pua and CONTROLS[unicode_pua] then
       	  CONTROLS[unicode_pua]()
	  return
       end
    end

    local unicode_char = unicode_int ~= 0 and string.char(unicode_int) or nil
    if CONTROLS and CONTROLS[unicode_char] then
        CONTROLS[unicode_char]()
    end
end