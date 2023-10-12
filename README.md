# Team19-BulletBrawl
## Milestone 1

In this milestone, we implemented the basic gameplay features outlined in our initial development plan. We have basic collision between the player and the platform, as well as the movement and related physics implemented. The player can jump (and double jump) between the platforms, and can drop through if the “down” key is pressed. The player sprite flips based on the direction the player is facing, and the player can shoot or throw a projectile in that direction using F and P (these keybindings are subject to change).  We have also met the goals of adding assets such as textured geometry for the player sprites and implemented a test map. There are no drastic changes and all requirements for this milestone have been met so no update to the proposal is needed.


- Basic collision between rectangular player(s) and platform [PhysicsSystem::step() 118-153, PhysicsSystem::collides()]
  - Implemented one way collisions, such that players do not collide with platforms when entering platforms from below but do collide with platforms when colliding from above [WorldSystem::step() line 161-173]
  - Players are able to stand on platforms [WorldSystem::handle_collisions() line 279]
- Player movement [WorldSystem::on_key(), PhysicsSystem:step()]
  - Left and right with friction [PhysicsSystem:step() line 62 - 96]
  - Jump and gravity [PhysicsSystem:step() line 102 - 113]
  - Press down to drop through platform  [WorldSystem::on_key() line 448-452]
- Press button to shoot and throw projectile [WorldSystem::on_key() line 325-339]
- Basic map with 4 platforms at different levels [WorldSystem::restart_game() line 240-242]
- Player respawn if they leave the map boundaries [WorldSystem::step() line 176-187]
- Player sprite that reflects player orientation [RenderSystem::drawTexturedMesh() line 19-25]
- Shooting projectile has recoil (player is pushed in direction opposite to projectile) [PhysicsSystem::step() line 45-52 ]
