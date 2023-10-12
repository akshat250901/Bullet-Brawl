# Team19-BulletBrawl
## Milestone 1
- Basic collision between player(s) and platform [WorldSystem::handle_collisions()]
  - Implemented one way collisions, such that players do not collide with platforms when entering platforms from below but do collide with platforms when colliding from above 
  - Players are able to stand on platforms
- Player movement [WorldSystem::on_key()]
  - Left and right with friction
  - Jump and gravity
  - Press down to drop through platform
- Press button to shoot and throw projectile [WorldSystem::on_key()]
- Basic map with 4 platforms at different levels [WorldSystem::restart_game()]
- Player respawn if they leave the map boundaries [WorldSystem::step()]
- Player sprite that reflects player orientation [RenderSystem::drawTexturedMesh()]
- Shooting projectile has recoil (player is pushed in direction opposite to projectile)
