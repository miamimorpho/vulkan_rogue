local vkterm = require("lua/vkterm_lib")
local rogue = require("lua/rogue_lib")

CONTROLS = {
      ['w'] = function(object) rogue.moveMobile(object, 0, -1) end,
      ['s'] = function(object) rogue.moveMobile(object, 0, 1) end,
      ['a'] = function(object) rogue.moveMobile(object, -1, 0) end,
      ['d'] = function(object) rogue.moveMobile(object, 1, 0) end,
      ['b'] = function(object) 
              local tile_picker = require("lua/tile_picker")
              vkterm.runGui(tile_picker)
            end,       
      ['left_click'] = function(object, dx, dy) rogue.moveMobile(object, dx, dy) end
}

-- Function to handle input
function handleInput(OBJECT_PTR)

    local cast_object = rogue.castGameObject(OBJECT_PTR)
    local unicode_int = vkterm.getUnicode()

    -- Mouse Controls --
    if unicode_int >= vkterm.PUA_START then

       local mouse_x = vkterm.mouseX()
       local mouse_y = vkterm.mouseY()
       local dx = 0
       local dy = 0
       
       if mouse_x > 0.55 then dx = 1 end
       if mouse_x < 0.45 then dx = -1 end
       if mouse_y > 0.55 then dy = 1 end
       if mouse_y < 0.45 then dy = -1 end

       local unicode_pua = vkterm.PUA_CODES[unicode_int]
       if unicode_pua and CONTROLS[unicode_pua] then
       	  CONTROLS[unicode_pua](cast_object, dx, dy)
       end
       return
    end

    -- Keyboard Controls --
    local unicode_char = unicode_int ~= 0 and string.char(unicode_int) or nil
    if CONTROLS and CONTROLS[unicode_char] then
        CONTROLS[unicode_char](cast_object)
    end
end