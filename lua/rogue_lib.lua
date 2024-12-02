local ffi = require("ffi")
ffi.cdef[[
    struct GameObject;
    typedef struct GameObject GameObject;

    int rogueMoveMobile(GameObject* object_ptr, int dx, int dy);
]]

local rogue = {}

rogue.moveMobile = ffi.C.rogueMoveMobile

function rogue.castGameObject(OBJECT_PTR)
    if not OBJECT_PTR then
        error("No current game object set")
        return nil
    end
    return ffi.cast("GameObject*", OBJECT_PTR)
end

return rogue