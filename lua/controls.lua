-- input_config.lua

local ffi = require("ffi")

ffi.cdef[[
    struct GameObject;
    typedef struct GameObject GameObject;

    double lMouseX(void);
    double lMouseY(void);
    int lGetUnicode(void);
    int lMoveMobile(GameObject* object_ptr, int dx, int dy);
]]

-- Define input mappings

local UNICODE_PUA_START = 0xE000  -- Start of Private Use Area
local UNICODE_PUA_END = 0xF8FF   -- End of Private Use Area

-- Mouse button mappings in PUA
local MOUSE_BUTTON_CODES = {
    [UNICODE_PUA_START + 0] = 'left_click',
    [UNICODE_PUA_START + 1] = 'right_click',
    [UNICODE_PUA_START + 2] = 'middle_click',
    [UNICODE_PUA_START + 3] = 'scroll_up',
    [UNICODE_PUA_START + 4] = 'scroll_down'
}

local CONTROLS = {
      ['w'] = function() ffi.C.lMoveMobile(game_object, 0, -1) end,    -- Move up
      ['s'] = function() ffi.C.lMoveMobile(game_object, 0, 1) end,     -- Move down
      ['a'] = function() ffi.C.lMoveMobile(game_object, -1, 0) end,    -- Move left
      ['d'] = function() ffi.C.lMoveMobile(game_object, 1, 0) end,     -- Move right
      ['left_click'] = function(dx, dy) ffi.C.lMoveMobile(game_object, dx, dy) end
}

-- Function to handle input
function handleInput()

    if not CURRENT_GAME_OBJECT then
        error("No current game object set")
    end
    game_object = ffi.cast("GameObject*", CURRENT_GAME_OBJECT)
    
    local unicode_int = ffi.C.lGetUnicode()

    -- Mouse Controls --
    if unicode_int >= UNICODE_PUA_START then

       local mouse_x = ffi.C.lMouseX()
       local mouse_y = ffi.C.lMouseY()
       local dx = 0
       local dy = 0
       
       if mouse_x > 0.55 then dx = 1 end
       if mouse_x < 0.45 then dx = -1 end
       if mouse_y > 0.55 then dy = 1 end
       if mouse_y < 0.45 then dy = -1 end

       local unicode_pua = MOUSE_BUTTON_CODES[unicode_int]
       if unicode_pua and CONTROLS[unicode_pua] then
       	  CONTROLS[unicode_pua](dx, dy)
       end
       return
    end

    -- Keyboard Controls --
    local unicode_char = unicode_int ~= 0 and string.char(unicode_int) or nil
    if CONTROLS and CONTROLS[unicode_char] then
        CONTROLS[unicode_char]()
    end
end