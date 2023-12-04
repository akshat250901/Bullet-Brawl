# Team19-BulletBrawl

Trailer: https://www.youtube.com/watch?v=HrgWrEJod6Q

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

## Milestone 3
In Milestone 3, the major milestone was successfully implementing the Gun System, adding various weapons each supporting a unique playstyle and with their own sounds and animations. We integrated sound effects, UI elements for power-ups/weapons, and diverse art assets. In addition, we focused on map refactoring, creating visually appealing maps like Island, Jungle, Space, and Temple. Overall, we enhanced the user experience and quality of life, by introducing an enhanced main menu intro screen and added clickable buttons to allow players to choose between various maps. Furthermore, we added text to indicate when players are reloading and arrows to indicate when players are outside of the view along with satisfying victory screens. Lastly, we rebalanced and fine-tuned the gameplay by tuning the guns and powerups. All requirements for this milestone have been met, and no major changes are needed based on the initial proposal.

Danny:
- Implement the Gun System which includes all the game logic that surrounds all the guns
  - Add gun declarations and stats to a list so that they can be spawned in weapon crates [RandomDropsSystem::RandomDropsSystem()]
  - Gun recoil animation [GunSystem::animateRecoil()]
  - Gun logic for all weapons [GunSystem::step()]
  - Damage drop off for weapons [WorldSystem::handle_player_bullet_collisions() lines 516-529]
- Implemented arrows that point to players' positions when they are outside of the screen [OutOfBoundsArrowSystem::step()]

Leo
- Implemented sound effects for player movement and guns [sound_system.cpp and sound_system.hpp]
- Implemented UI elements to display what powerup/weapon is being picked up, as well as if the player is currently reloading [world_init.cpp::createPopupIndicator()]
- Various art assets like the main menu, guns, popup indicators, win screen[/data/*]
  - Gun renders different depending on the weapon that the player is holding [world_init.cpp: line 378-413]

Jason
- Refactored createBackground() methods for different maps
  - [world_init::createIslandMap()]
  - [world_init::createJungleMap()]
  - [world_init::SpaceMap()]
  - [world_init::TempleMap()]
- Created art assets like Space, Jungle and Temple map for more interesting game play
- Created map layout to accommodate new game play maps


Seraj:
- Created end game screen which shows the winning player and added fade out animation
  - world_init.cpp#L712 [createDeathScreen]
- Added stock count for player lives
  - world_system.cpp#L118[WorldSystem::step]

Akshat:
- Implemented a new rocket System
  - [rocket_system.cpp]
  - [rocket_system.hpp]
- Created a rocket for the players to shoot another player
  - [world_init::createRocket()]
- Implemented complex prescribed motion specifically Bezier S motion for the rocket’s initial motion
  - [rocket_system:: updateBezierMotion()]
  - [rocket_system::interpolate()]
- Implemented path finding for rocket so that it moves towards the other player and tries to hit it
  - [rocket_system::updatePathfinding()]

Ishraq:
- Added the level selection screen by adding another layer to MainMenuSystem
- Updated menu selection to allow mouse clicks to switch to game modes and switch between main menu and level selection
  - [MainMenuSystem::initialize_main_menu()]
  - [MainMenuSystem::is_point_inside_button()]
  - [MainMenuSystem::is_hovering()]
  - [MainMenuSystem::on_mouse_move()]
- Updated player respawn positions to be random
  - [WorldSystem::step()]

## Milestone 3

In this final milestone, we integrated an external software library (GLText) for text rendering, allowing us to dynamically display key information such as lives, ammo count and current weapons through UI elements. On the visual front, we tackled platform art layout issues and fixed the player-winning screen to display the correct winner. There were improvements in stability with fixes for potential crashes related to mouse click handling. We also added a story to include some narrative in the game experience. In addition, we updated assets across menus, maps, and backgrounds refreshing the maps for more cleaner and aesthetically pleasing visuals. Some other rendering changes include an electric touch for powerups. Lastly, various balance changes were made to the guns and powerups while also introducing an initial invincibility upon player respawning to ensure fair and balanced gameplay.

Danny:
- Incorporated external software library text rendering to render text in our game
  - Draw text in render system [RenderSystem::drawText()]
  - Handle dynamic text resizing based on screen dimensions [RenderSystem::drawText() lines 339-360]
  - Created wrapped that allows for ease of text rendering [WorldInit::createText()]
- Implemented text showing then fading to highlight when the player dies [WorldSystem::step() lines 226-254]
- Implemented UI elements to show ammo count and current gun for each player [WorldSystem::restart_game() lines 339-348, GunSystem::step() lines 121-133]
- Readjust friction in the game to apply on every loop for consistency in knockback [PhysicsSystem::step() 260-277]

Jason:
- Fixed the platform art layout issue with the temple map
- Fixed player winning screen always showing red player when winning [PlayerRespawnSystem::step() lines 45-50]

Leo:
- Implemented Story System [story_system.cpp/story_system.hpp]
- Fixed mouse click handler nullptr that would crash the game if we clicked in-game [input_system.cpp/input_system.hpp]
- Updated tutorial level [WorldInit::createTutorialMap line 754 + multiple places that check if game_state_system->get_current_state() == 3]
- Updated various assets including, art and sounds.[Main menu, maps, bgm, story frames]
- Created short trailer video


Seraj:
- Implemented text showing then fading to highlight when the player dies [WorldSystem::step() lines 226-254]
- Implemented bullet prediction effect so that bullets light up with the shooter color when it’s predicted to hit the other player[PhysicsSystem::predictCollisionBetweenPlayerAndBullet()]
- Implemented electric effect for bullets and powerups

Ishraq:
- Worked on assisting other teammates with their tasks, such as adding buttons to level selection screen [MainMenuSystem::initializeMainMenu()]
- Wrote documentation and completed the ECS diagram
- Added initial invincibility to players when respawning [PlayerRespawnSystem::step()] [WorldSystem::step()]


## Report for changes in response to feedback:

There were numerous changes made in response to feedback from users during the crossplay sessions and also from data collected from friends who were asked to test out the game. I have broken down the changes into user experience changes, including menus UI and other changes not related to gameplay or gameplay-specific balance changes.

**User experience changes:**

A friend mentioned the main menu art felt too plain and that it misrepresented our game thus we improved the look of the main menu by introducing higher-quality pixel art in the background including the silhouette of one of our characters.

From M2 crossplay we received a comment about how dying could have a better indication. To address this we make it so that once a player dies text pops up and shows which player has lost a life, furthermore we have added a sound effect to indicate the player's death.

During M2 crossplay another player commented that we could add a visual indication when reloading. Thus, we added a text element attached to the player to indicate that a player is reloading and the text element disappears when the player stops reloading. Furthermore, it enhances the experience when the player picks up elements. We also added similar text elements to indicate item pickups such as picking up mystery boxes and powerups to more clearly indicate exactly what was picked up.

Another player commented that the tutorial needed improvement and we changed this by re-doing the art and map design such it was a simpler layout more easily accessible to beginner players and included various text descriptions explaining how each powerup or gun works. We also made it so that guns and powerups can be picked up at any time so players can experiment with the guns and powerups infinitely with a dummy player as well.

Lastly, a friend commented on how the hearts to indicate lives in the top right and left of the screens sometimes obscured the gameplay, in this case, we refactored the health counter and included an ammo counter to help players keep track of the ammo in their gun which was a general comment people had in the crossplay session as well as they had no idea how much ammo was left.

**Gameplay balance changes:**

One player in M2 commented that occasionally the knockback of the guns felt too strong, this was an intentional mechanic such that knockback would be extremely hard if the player getting hit was going in the same direction as the bullet, this was done initially to introduce a skill gap into the game. However, as seen from the crossplay session most players did not understand the mechanic and thus had a difficult time comprehending why the knockback was inconsistent. Thus, this was removed and a consistent knockback was implemented instead.

A player in M2 mentioned that the powerups felt underwhelming and this was addressed by improving the powers granted by powerups across the board. Speed boost was drastically increased and jump boost along with triple jump and their durations increased. This still felt underwhelming as during playtests with friends I noticed players still were not picking up powerups. In this case, I added a 30% knockback resistance to every powerup so that players would be more incentivized to pick up powerups.

Lastly, various comments in M2 mentioned how the game felt like it lasted forever and dragged on too long. To address this we increased the knockback of all guns across the board, we also reduced the gun model sizes of the sniper and shotgun to decrease the distance where the bullet spawns. Overall the knockback increases made it such that it was far easier to get knocked off the platforms compared to before increasing the pace and tension of the game. This made it so that it was far more likely to kill the other player and thus rounds became shorter and fixed the issue of games feeling like they lasted too long. 
