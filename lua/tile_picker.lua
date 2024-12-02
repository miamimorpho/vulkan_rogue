local rogue = require("lua/rogue_lib")

local gui_file = {
    { -- list member is a label
        text = "Hello, World!",       
        x = 0,
        y = 0,
        fg = 15,
	    bg = 0,

	    on_hover = function(self)        
            self.bg = 4
	    end,
	
        on_click = function(self)
            print("Label clicked!")
        end
    },
}

return gui_file