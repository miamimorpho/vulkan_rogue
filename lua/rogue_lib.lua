local ffi = require("ffi")
ffi.cdef[[
    struct GameObject;
    typedef struct GameObject GameObject;

    int rogueMoveMobile(GameObject* object_ptr, int dx, int dy);
    int roguePaintObject(GameObject* object_ptr, uint16_t inventory_index, uint16_t unicode, uint16_t atlas, uint16_t fg, uint16_t bg);
    int rogueBuildObject(GameObject* object_ptr, uint16_t inventory_index);

]]

local rogue = {}

rogue.moveMobile = ffi.C.rogueMoveMobile
rogue.paintObject = ffi.C.roguePaintObject
rogue.buildObject = ffi.C.rogueBuildObject

function rogue.castGameObject(OBJECT_PTR)
    if not OBJECT_PTR then
        error("No current game object set")
        return nil
    end
    return ffi.cast("GameObject*", OBJECT_PTR)
end

return rogue