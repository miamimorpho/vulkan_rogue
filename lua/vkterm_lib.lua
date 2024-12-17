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
    void gfxClear(Gfx);

    int gfxRenderGlyph(Gfx, uint16_t, uint16_t, uint16_t, 
                       uint16_t, uint16_t, uint16_t);
    int gfxRenderElement(Gfx, uint16_t x, uint16_t y,
                         const char* str,
                         uint16_t atlas_index,
		                 uint16_t fg, uint16_t bg);
    int gfxLayerChange(Gfx, const char*);
    int gfxLayerPresent(Gfx, const char*);
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

vkterm.clear = function(name) ffi.C.gfxClear(gfx_cast_ptr) end

vkterm.layerChange = function(name) return ffi.C.gfxLayerChange(gfx_cast_ptr, name) end
vkterm.layerPresent = function(name) return ffi.C.gfxLayerPresent(gfx_cast_ptr, name) end
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

function vkterm.renderGlyph(x, y, encoding, atlas_index, fg, bg) 
      ffi.C.gfxRenderGlyph(gfx_cast_ptr,
        x or 0,
        y or 0,
        encoding or 0,
        atlas_index or 1,
        fg or 15,
        bg or 0 ) 
end

function vkterm.renderElement(element)
      ffi.C.gfxRenderElement(gfx_cast_ptr,
        element.x or 0,
        element.y or 0, 
        element.text or " ", 
        element.atlas or 1, 
        element.fg or 15, 
        element.bg or 0)
end

local function renderStaticElements(elements_src)

    for i, e in pairs(elements_src) do
        e.width = e.width or string.len(tostring(e.text or " "))
        e.height = e.height or 1
        if not e.on_draw then
            vkterm.renderElement(e)
        else 
            e:on_draw()
        end
    end
end

local function runObservers(elements)

    vkterm.clear()

    local mouse_x = vkterm.mouseX()
    local mouse_y = vkterm.mouseY()
    local screen_width = vkterm.screenWidth()
    local screen_height = vkterm.screenHeight()

    for _, element in pairs(elements) do

        local in_bounds = 
        
            mouse_x > element.x / screen_width and 
            mouse_x < (element.x + element.width) / screen_width and
            mouse_y > element.y / screen_height and 
            mouse_y < (element.y + element.height) / screen_height
        
        -- Handle hover state
        if in_bounds then

            mouse_x_unorm = math.floor(mouse_x * screen_width)
            mouse_y_unorm = math.floor(mouse_y * screen_height)

            if element.on_hover then
                element:on_hover(mouse_x_unorm, 
                                 mouse_y_unorm)
            end
            
            -- Check for click
            if vkterm.isMouseClicked() and element.on_click then
                element:on_click(mouse_x_unorm, mouse_y_unorm)
            end
        end
       
        if element.to_exit then return element.return_value end
    end
    return 0
end

function vkterm.runGui(gui_filename)

    local file, err = loadfile(gui_filename)
    if file then
        elements = file()
    else
        error("Could not load file: " .. tostring(err))
    end

    vkterm.layerChange(gui_filename)

    renderStaticElements(elements) 
    vkterm.layerChange("fg")   

    local res = 0;
    while true do
      vkterm.pollEvents()
      res = runObservers(elements)
      if res ~= 0 then
         break
      end 
  
      vkterm.layerPresent(gui_filename)
      vkterm.layerPresent("fg")
      vkterm.refresh()
    end
    vkterm.layerChange("main")
    return res
end

return vkterm