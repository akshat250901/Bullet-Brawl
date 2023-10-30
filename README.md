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

## Milestone 2
In this milestone, we've achieved substantial advancements by introducing new features and enhancing existing ones. Our efforts have led to the creation of a welcoming user interface with a tutorial, ensuring a smoother and more engaging start to the game. The addition of sprites for players and power-ups has injected life and character into the gameplay, while responsiveness and power-up functionalities have been fine-tuned for a more dynamic experience. We've also boosted the game's visual appeal through captivating parallax effects and a platform texture map, enriching the immersive environment. The inclusion of collisions between players and bullets/projectiles adds realism and strategy to the gameplay, while the implementation of key frame/state interpolation ensures seamless transitions, offering players a more polished and enjoyable gaming experience. There are no drastic changes and all requirements for this milestone have been met so no update to the proposal is needed.

Leo:
- Sprite sheet animations
  - Animated sprite shader [animated.fs.glsl/animated.vs.glsl]
  - Similar code inside player shader [player.fs.glsl/player.vs.glsl]
- Animated sprite component [components.hpp line 64-75]
- Animation system [AnimationSystem.cpp/.hpp]
  - Player [AnimationSystem::step line 7-43]
  - Power ups [AnimationSystem::step line 45-70]
- Code in render system to handle animations [RenderSystem::drawAnimated()]
- Sprite art [player_spritesheet.png/powerups_spritesheet.png]
- Refactored createPlayer to tint players a certain color, instead of needing sprite sheets in multiple colors [WorldSystem::spawn_player()]

Danny:
- Game logic response to user input
  - Implemented random power up spawns powerups [RandomDropsSystem::step()]
  - Power ups increase player stats on collision with player [WorldSystem::handle_player_powerup_collisions()]
  - When power up timer expires, remove stat bonuses [WorldSystem::step() lines 178-206]
- Refactoring to increase system extensibility
  - Refactored movement into movement system that handles all player key inputs [MovementSystem::step()]
  - Refactored collision detection such that only we have fine grained control over the collisions [WorldSystem::handle_collision()], [PhysicsSystem::step() line 294-296]

Jason:
- Parallax Effect
  - created background asset fragment shader [shaders/background.fs.glsl]
  - created background asset vertex shader [shaders/background.vs.glsl]
  - implemented rendering background logic [render_system::drawTexturedMesh() line 165-195]
  - created background background layer entity [world_init::createBackgroundBack() line 188-212]
  - created background middle layer entity [createBackgroundMiddle() line 214-239]
  - created background foreground layer entity [createBackgroundForeground() line 241-269]
  - implemented setting scrolling speed in [WorldSystem::step() line207-222]
- Platform texture map
  - method to create the island background [textureworld_init::createBackgroundIsland()]line155-186
  - WorldSystem::restart_game() line254

Seraj:
- Collision between player and bullet
	- Having meshes for bullets and projectiles [./data/meshes]
	- Mesh collision between player and bullets/projectiles [PhysicsSystem::meshIntersectsMotion]
	- Projectiles made go further when player is moving [world_init.cpp:L101]

Akshat: 
- Key Frame/State interpolation
  - Created a new component and component container for interpolation  [Components.hpp 76 - 83, Tiny_ecs_registry.hpp]
  - Added interpolation to powerups with desired values[World_init.cpp 136-141 ]
  - Handled interpolation functioning for powerup [random_drops_system.cpp 59- 80]
