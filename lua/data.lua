textures = {
  "color.png",
  "icl8x8u.bdf",
  "mrmotext-ex11.png"
}

terrain = {
  air = {
    unicode = 0,
    texture = "mrmotext-ex11.png",
    fg = 15,
    bg = 3
  },

  wall = {
    unicode = 370,
    texture = "mrmotext-ex11.png",
    fg = 3,
    bg = 0,
    blocks_vision = true,
    blocks_sight = true
-- blocks_vision and blocks_sight are stored as a 1-bit mask in the terrain chunk, though it is initialised as an entity.
  }
}

monsters = {
  player = {
    unicode = 100,
    texture = "mrmotext-ex11.png",
    fg = 15,
    bg = 0,
    hp = 15,
    atk = 5
  },

  goblin = {
    unicode = 50,
    texture = "mrmotext-ex11.png",
    fg = 3,
    bg = 0,
    hp = 4,
    atk = 4
  }

}