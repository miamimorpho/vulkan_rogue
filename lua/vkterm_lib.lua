-- C HEADER
local ffi = require("ffi")
ffi.cdef[[

    struct GfxGlobal;
    typedef struct GfxGlobal* Gfx;

    void gfxPollEvents(Gfx);
    double gfxMouseX(void);
    double gfxMouseY(void);
    int gfxGetKey(void);
    int gfxGetScreenWidth(Gfx);
    int gfxGetScreenHeight(Gfx);
    int gfxRenderElement(Gfx, uint16_t x, uint16_t y,
                         const char* str,
                         uint16_t atlas_index,
		                 uint16_t fg, uint16_t bg);
    int gfxRefresh(Gfx);
]]

if not GFX_IMPL then
   error("No GFX_IMPL set from C side")
else
   gfx_cast_ptr = ffi.cast("Gfx", GFX_IMPL)
end

local vkterm = {}
vkterm.pollEvents = function() ffi.C.gfxPollEvents(gfx_cast_ptr) end
vkterm.mouseX = ffi.C.gfxMouseX
vkterm.mouseY = ffi.C.gfxMouseY
vkterm.getUnicode = ffi.C.gfxGetKey

vkterm.screenWidth = function() return ffi.C.gfxGetScreenWidth(gfx_cast_ptr) end
vkterm.screenHeight = function() return ffi.C.gfxGetScreenHeight(gfx_cast_ptr) end
vkterm.refresh = function() return ffi.C.gfxRefresh(gfx_cast_ptr) end

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
    return false
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

local function compileElement(options)
    return {
        text = options.text or " ",
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

local function runElementTable(elements)

    local mouse_x = vkterm.mouseX()
    local mouse_y = vkterm.mouseY()

    for _, element_raw in ipairs(elements) do
        local element = compileElement(element_raw)

        local in_bounds = 
            mouse_x > element.x / vkterm.screenWidth() and 
            mouse_x <= (element.x + element.width) / vkterm.screenWidth() and
            mouse_y > element.y / vkterm.screenHeight() and 
            mouse_y <= (element.y + element.height) / vkterm.screenHeight()
        
        -- Handle hover state
        if in_bounds then
            if element.on_hover then
                element:on_hover()
            end
            
            -- Check for click
            if vkterm.isMouseClicked() and element.on_click then
                element:on_click()
            end
        end
        
        renderElement(element)
        if element.to_exit then return 1 end
    end
    return 0
end

function vkterm.runGui(elements_table)
    while(runElementTable(elements_table) == 0) do 
      vkterm.pollEvents()
      vkterm.refresh()
    end
end

return vkterm