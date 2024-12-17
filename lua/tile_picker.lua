local rogue = require("lua/rogue_lib")

local ATLAS_WIDTH = 32
local GLYPH_VIEWPORT_SIZE = 16

local g_target_fg = 0
local g_target_bg = 0
local g_target_encoding = 0
local g_atlas_page = 0

local function get_return_value()
     return { 
     0, -- invetory index --
     g_target_encoding, -- unicode --
     2, -- atlas index --
     g_target_fg, -- foreground colour --
     g_target_bg -- background colour --
     }
end

local function glyph_picker_page_draw(self)
   
   o_x = 2
   o_y = 8

   local page_x_offset = math.modf(g_atlas_page * GLYPH_VIEWPORT_SIZE, 2)
   local page_y_offset = math.floor(g_atlas_page / 2) * (GLYPH_VIEWPORT_SIZE - 1)

   for x = 0, 15, 1 do
   for y = 0, 15, 1 do
       local encoding = ((y + page_y_offset) * ATLAS_WIDTH) + x + page_x_offset
       vkterm.renderGlyph(o_x + x, o_y + y,
                          encoding, 2, 15, 0)
   end
   end
end

local tile_picker = {
    fg_title = { -- Foreground Color Picker Title --
        text = "Foreground Color",       
        x = 2,
        y = 1,
        atlas = 1,
        fg = 15,
	    bg = 0,
    },
    fg_picker = { -- fg Color Picker --
        x = 2,
        y = 2,
        atlas = 1,
        width = 16,
        height = 1,
    
        on_draw = function(self)
            for i = 0, 16, 1 do
                vkterm.renderGlyph(self.x + i, self.y, 0, 2, i, i)
            end
        end,

	    on_hover = function(self, mouse_x, mouse_y)
            vkterm.renderGlyph(mouse_x, mouse_y, 0, 2, 15, 15)
	    end,
	
        on_click = function(self, mouse_x, mouse_y)
            g_target_fg = mouse_x - self.x
        end
    },
    bg_title = { -- Background Color Picker Title --
        text = "Background Color",       
        x = 2,
        y = 4,
        atlas = 1,
        fg = 15,
	    bg = 0,
    },
    bg_picker = { -- bg Color Picker --
        x = 2,
        y = 5,
        atlas = 1,
        width = 16,
        height = 1,
    
        on_draw = function(self)
            for i = 0, 15, 1 do
                vkterm.renderGlyph(self.x + i, self.y, 0, 2, i, i)
            end
        end,

	    on_hover = function(self, mouse_x, mouse_y)
            vkterm.renderGlyph(mouse_x, mouse_y, 0, 2, 15, 15)
	    end,
	
        on_click = function(self, mouse_x, mouse_y)
            g_target_bg = mouse_x - self.x
        end
    },
    glyph_picker_title = { -- Glyph Picker Title --
        text = "Glyph",       
        x = 2,
        y = 7,
        atlas = 1,
        fg = 15,
	    bg = 0,
    },
    glyph_picker_back_button = {
        text = "<",
        x = 15,
        y = 7,
        atlas = 1,
        fg = 3,
        bg = 12,

	    on_hover = function(self, mouse_x, mouse_y)
            vkterm.renderGlyph(mouse_x, mouse_y, 0, 2, 12, 3)
	    end,

        on_click = function(self)
           g_atlas_page = math.fmod(g_atlas_page -1, 4)
           if g_atlas_page < 0 then g_atlas_page = 3 end

           vkterm.layerChange("lua/tile_picker.lua")
           glyph_picker_page_draw()
           vkterm.layerChange("fg")
        end
    },
    glyph_picker_next_button = {
        text = ">",
        x = 17,
        y = 7,
        atlas = 1,
        fg = 3,
        bg = 12,

	    on_hover = function(self, mouse_x, mouse_y)
            vkterm.renderGlyph(mouse_x, mouse_y, 0, 2, 12, 3)
	    end,

        on_click = function(self)
           g_atlas_page = math.fmod(g_atlas_page +1, 4)
           vkterm.layerChange("lua/tile_picker.lua")
           glyph_picker_page_draw()
           vkterm.layerChange("fg")
        end
    },
    glyph_picker_pad = { -- glyph picker --
   
        x = 2,
        y = 8,
        width = 16,
        height = 16,
     
        on_draw = function(self)
           glyph_picker_page_draw(self)
        end,

     	on_hover = function(self, mouse_x, mouse_y)
            vkterm.renderGlyph(mouse_x, mouse_y, 0, 2, 3, 3)
	    end,
	
        on_click = function(self, mouse_x, mouse_y)

            local mouse_x_local = mouse_x - self.x
            local mouse_y_local = mouse_y - self.y

            local page_x_offset = 
                  math.modf(g_atlas_page * GLYPH_VIEWPORT_SIZE, 2)
            local page_y_offset =
                  math.floor(g_atlas_page / 2) * (GLYPH_VIEWPORT_SIZE - 1)
            g_target_encoding = ((mouse_y_local + page_y_offset) * ATLAS_WIDTH) + page_x_offset + mouse_x_local
            print(g_target_encoding)
        end
    },
    done = {
        text = "done",
        x = 20,
        y = 7,
        fg = 3,
        bg = 12,

	    on_hover = function(self, mouse_x, mouse_y)
            vkterm.renderGlyph(mouse_x, mouse_y, 0, 2, 12, 3)
	    end,

        on_click = function(self)
           self.return_value = get_return_value()
           self.to_exit = 1
        end
    }
}

return tile_picker