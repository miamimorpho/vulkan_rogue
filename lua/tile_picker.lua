local rogue = require("lua/rogue_lib")

local gui_file = {
    { -- list member is a label
        text = "Foreground Color",       
        x = 0,
        y = 0,
        atlas = 1,
        fg = 15,
	    bg = 0,

	    on_hover = function(self)        
            self.bg = 4
	    end,
	
        on_click = function(self)
            self.to_exit = true
        end
    },
    { -- list member is a label
        text = "Background Color",       
        x = 0,
        y = 3,
        atlas = 1,
        fg = 15,
	    bg = 0,

	    on_hover = function(self)        
            self.bg = 4
	    end,
	
        on_click = function(self)
            self.to_exit = true
        end
    },
}

return gui_file