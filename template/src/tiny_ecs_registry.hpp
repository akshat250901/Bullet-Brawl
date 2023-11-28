#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<Friction> friction;
	ComponentContainer<Gravity> gravity;
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;

	// Collision containers
	ComponentContainer<PlayerPlatformCollision> playerPlatformCollisions;
	ComponentContainer<PlayerPlatformCollision> playerPowerUpCollisions;
	ComponentContainer<PlayerBulletCollision> playerBulletCollisions;
	ComponentContainer<PlayerMysteryBoxCollision> playerMysteryBoxCollisions;

	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Platform> platforms;
	ComponentContainer<Bullet> bullets;
	ComponentContainer<PlayerStatModifier> playerStatModifiers;
	ComponentContainer<PowerUp> powerUps;
	ComponentContainer<Interpolation> interpolation;
	ComponentContainer<AnimatedSprite> animatedSprite;
	ComponentContainer<ParallaxBackground> parallaxes;
	ComponentContainer<Controller> controllers;
	ComponentContainer<Life> lives;
	ComponentContainer<Gun> guns;
	ComponentContainer<GunMysteryBox> gunMysteryBoxes;
	ComponentContainer<NonInteractable> nonInteractables;
	ComponentContainer<MuzzleFlash> muzzleFlashes;
	ComponentContainer<OutOfBoundsArrow> outOfBoundsArrows;
	ComponentContainer<Invincibility>invincibility;
	ComponentContainer<Text> texts;
	ComponentContainer<TextDeathLog> deathLog;

	ComponentContainer<PopupIndicator> popupIndicator;
	// ComponentContainer<Button> buttons;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&friction);
		registry_list.push_back(&gravity);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&platforms);
		registry_list.push_back(&bullets);
		registry_list.push_back(&playerStatModifiers);
		registry_list.push_back(&powerUps);
		registry_list.push_back(&animatedSprite);
		registry_list.push_back(&parallaxes);
		registry_list.push_back(&controllers);
		registry_list.push_back(&lives);
		registry_list.push_back(&guns);
		registry_list.push_back(&gunMysteryBoxes);
		registry_list.push_back(&nonInteractables);
		registry_list.push_back(&popupIndicator);
		registry_list.push_back(&muzzleFlashes);
		registry_list.push_back(&outOfBoundsArrows);
		registry_list.push_back(&invincibility);
		registry_list.push_back(&texts);
		registry_list.push_back(&deathLog);

		// Collisions
		registry_list.push_back(&playerPlatformCollisions);
		registry_list.push_back(&playerPowerUpCollisions);
		registry_list.push_back(&playerBulletCollisions);
		registry_list.push_back(&playerMysteryBoxCollisions);

		// registry_list.push_back(&buttons);
		registry_list.push_back(&interpolation);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;