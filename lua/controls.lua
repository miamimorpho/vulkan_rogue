-- input_config.lua
-- Define input mappings
local inputBindings = {
      ['w'] = function() moveMobile(0, -1) end,    -- Move up
      ['s'] = function() moveMobile(0, 1) end,     -- Move down
      ['a'] = function() moveMobile(-1, 0) end,    -- Move left
      ['d'] = function() moveMobile(1, 0) end,     -- Move right
}

-- Function to handle input
function handleInput(key, mouse_x, mouse_y)
    if inputBindings and inputBindings[key] then
        inputBindings[key]()
    end
end