-- C HEADER
local ffi = require("ffi")
ffi.cdef[[

    struct GfxGlobal;
    typedef struct GfxGlobal* Gfx;

    void gfxPollEvents(Gfx);
    double gfxMouseX(void);
    double gfxMouseY(void);
    int gfxGetKey(void);
    int gfxRenderElement(Gfx, uint16_t x, uint16_t y,
                         const char* str,
                         uint16_t atlas_index,
		                 uint16_t fg, uint16_t bg);
    int gfxRefresh(Gfx);
]]

if not GFX_RAW_PTR then
   error("No GFX_RAW_PTR set from C side")
else
   gfx_cast_ptr = ffi.cast("Gfx", GFX_RAW_PTR)
end

local vkterm = {}
vkterm.pollEvents = function() ffi.C.gfxPollEvents(gfx_cast_ptr) end
vkterm.mouseX = ffi.C.gfxMouseX
vkterm.mouseY = ffi.C.gfxMouseY
vkterm.getUnicode = ffi.C.gfxGetKey
vkterm.refresh = function() ffi.C.gfxRefresh(gfx_cast_ptr) end

-- CONSTANTS --
vkterm.PUA_START = 0xE000
vkterm.PUA_CODES = {
    [vkterm.PUA_START + 0] = 'left_click',
    [vkterm.PUA_START + 1] = 'right_click',
    [vkterm.PUA_START + 2] = 'middle_click',
    [vkterm.PUA_START + 3] = 'scroll_up',
    [vkterm.PUA_START + 4] = 'scroll_down'
}
vkterm.PUA_END = 0xF8FF

function vkterm.isMouseClicked()
    local unicode_int = vkterm.getUnicode()
    if unicode_int >= vkterm.PUA_START then
       return true
    end
end

-- LUA ONLY GUI MODE --
local function renderElement(element)
      ffi.C.gfxRenderElement(gfx_cast_ptr,
        element.x,
        element.y, 
        element.text, 
        element.atlas, 
        element.fg, 
        element.bg)
end

local function newCstyleString(luaString)

    local buffer = ffi.new("char[?]", #luaString + 1)  -- +1 for null terminator
    ffi.copy(buffer, luaString)
    -- Null terminate the string
    buffer[#luaString] = 0  -- Set the last byte to 0

    return buffer
end

local function compileElement(options)
    if options.text then
       local text_c_string = newCstyleString(options.text)
    end

    return {
        text = text_c_string or " ",
        x = options.x or 0,
        y = options.y or 0,
        fg = options.fg or 4,
        bg = options.bg or 4,
        atlas = options.atlas or 0,
        width = string.len(tostring(options.text or " ")),
        height = options.height or 1,
        on_hover = options.on_hover or nil,
        on_click = options.on_click or nil
    }
end

local function compileElementTable(raw_elements)
    local compiled_table = {}
    for _, e in ipairs(raw_elements) do
        local compiled_e = compileElement(e)
        table.insert(compiled_table, compiled_e)
    end
    return compiled_table
end

local function runElementTable(elements)

    local mouse_x = vkterm.mouseX()
    local mouse_y = vkterm.mouseY()

    for _, element in ipairs(elements) do
        -- Check if mouse is within element bounds
        local in_bounds = 
            mouse_x >= element.x and 
            mouse_x <= (element.x + element.width) and
            mouse_y >= element.y and 
            mouse_y <= (element.y + element.height)
        
        -- Handle hover state
        if in_bounds then
            if element.on_hover then
                element:on_hover()
            end
            
            -- Check for click
            if vkterm.isMouseClicked() and element.onclick then
                element:onclick()
            end
        end
        
        renderElement(element)
    end
    return 0
end

function vkterm.runGui(elements_table)
         local dom = compileElementTable(elements_table)
         while(runElementTable(dom) == 0) do 

         end
end

return vkterm