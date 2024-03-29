#pragma once

#include "sound_system.hpp"
#include "tiny_ecs.hpp"

void SoundSystem::step(float elapsed_ms)
{
    // handle player movement related audio

    auto& players_container = registry.players;

    for (uint i = 0; i < players_container.components.size(); i++)
    {
        bool player_is_moving = players_container.components[i].is_running_left
            || players_container.components[i].is_running_right;
        bool player_on_ground = players_container.components[i].is_grounded;

        if (player_is_moving && player_on_ground) {
            play_walk_sound();
        }
    }
}

void SoundSystem::init_sounds() {
    printf("SOUNDS INITIALIZED\n");
    pistol_shoot_sound = Mix_LoadWAV(audio_path("guns/pistol_shoot.wav").c_str());
    smg_shoot_sound = Mix_LoadWAV(audio_path("guns/smg_shoot.wav").c_str());
    ar_shoot_sound = Mix_LoadWAV(audio_path("guns/ar_shoot.wav").c_str());
    sniper_shoot_sound = Mix_LoadWAV(audio_path("guns/sniper_shoot.wav").c_str());
    shotgun_shoot_sound = Mix_LoadWAV(audio_path("guns/shotgun_shoot.wav").c_str());

    player_reload_sound = Mix_LoadWAV(audio_path("guns/gun_reload.wav").c_str());
    weapon_pickup_sound = Mix_LoadWAV(audio_path("guns/weapon_pickup.wav").c_str());
    powerup_pickup_sound = Mix_LoadWAV(audio_path("powerup_pickup.wav").c_str());


    hit_sound = Mix_LoadWAV(audio_path("hit_sound.wav").c_str());

    player_step_sound = Mix_LoadWAV(audio_path("step.wav").c_str());

    player_fall_sound = Mix_LoadWAV(audio_path("player_fall.wav").c_str());

    background_burning = Mix_LoadMUS(audio_path("fire.wav").c_str());
    bgm = Mix_LoadMUS(audio_path("bgm.wav").c_str());

    int volume = 8;  // to save some ears
    Mix_VolumeChunk(pistol_shoot_sound, volume/2);
    Mix_VolumeChunk(smg_shoot_sound, volume);
    Mix_VolumeChunk(ar_shoot_sound, volume);
    Mix_VolumeChunk(sniper_shoot_sound, volume);
    Mix_VolumeChunk(shotgun_shoot_sound, volume);

    Mix_VolumeChunk(player_reload_sound, volume);
    Mix_VolumeChunk(weapon_pickup_sound, volume*6);
    Mix_VolumeChunk(powerup_pickup_sound, volume*2);

    Mix_VolumeChunk(hit_sound, volume * 2.5);

    Mix_VolumeChunk(player_step_sound, volume * 2);

    Mix_VolumeChunk(player_fall_sound, volume * 8);

    Mix_VolumeMusic(volume / 2);
}

void SoundSystem::play_shoot_sound(std::string gun_type) {
    if (gun_type == "SUBMACHINE GUN") {
        Mix_PlayChannel(-1, smg_shoot_sound, 0);
    }
    else if (gun_type == "ASSAULT RIFLE") {
        Mix_PlayChannel(-1, ar_shoot_sound, 0);
    }
    else if (gun_type == "SNIPER RIFLE") {
        Mix_PlayChannel(-1, sniper_shoot_sound, 0);
    }
    else if (gun_type == "SHOTGUN") {
        Mix_PlayChannel(-1, shotgun_shoot_sound, 0);
    }
    else {
        // default case should be pistol
        Mix_PlayChannel(-1, pistol_shoot_sound, 0);
    }
}

void SoundSystem::play_reload_sound(std::string gun_type) {
    Mix_PlayChannel(-1, player_reload_sound, 0);
}

void SoundSystem::play_pickup_sound(int type) {
    switch (type) {
        case 0: // weapon pickup
            Mix_PlayChannel(-1, weapon_pickup_sound, 0);
            break;
        case 1: // powerup pickup
            Mix_PlayChannel(-1, powerup_pickup_sound, 0);
            break;
        default:
            // do nothing
            break;
    }
   
}


void SoundSystem::play_walk_sound()
{
    if (!Mix_Playing(1)) {
        Mix_PlayChannel(1, player_step_sound, 0);
    }
}

void SoundSystem::play_fall_sound()
{
    
     Mix_PlayChannel(-1, player_fall_sound, 0);
    
}

void SoundSystem::play_hit_sound()
{
    Mix_PlayChannel(-1, hit_sound, 0);
}

void SoundSystem::play_burning_sound()
{
    Mix_PlayMusic(background_burning, -1);
}

void SoundSystem::play_bgm()
{
    Mix_PlayMusic(bgm, -1);
}


void SoundSystem::stop_music()
{
    Mix_HaltMusic();
}

SoundSystem::~SoundSystem() {
    if (pistol_shoot_sound != nullptr) {
        Mix_FreeChunk(pistol_shoot_sound);
        pistol_shoot_sound = nullptr; 
    }
    if (smg_shoot_sound != nullptr) {
        Mix_FreeChunk(smg_shoot_sound);
        smg_shoot_sound = nullptr;
    }
    if (ar_shoot_sound != nullptr) {
        Mix_FreeChunk(ar_shoot_sound);
        ar_shoot_sound = nullptr;
    }
    if (sniper_shoot_sound != nullptr) {
        Mix_FreeChunk(sniper_shoot_sound);
        sniper_shoot_sound = nullptr;
    }
    if (shotgun_shoot_sound != nullptr) {
        Mix_FreeChunk(shotgun_shoot_sound);
        shotgun_shoot_sound = nullptr;
    }

    if (player_reload_sound != nullptr) {
        Mix_FreeChunk(player_reload_sound);
        player_reload_sound = nullptr;
    }
    if (weapon_pickup_sound != nullptr) {
        Mix_FreeChunk(weapon_pickup_sound);
        weapon_pickup_sound = nullptr;
    }

    if (hit_sound != nullptr) {
        Mix_FreeChunk(hit_sound);
        hit_sound = nullptr;
    }
    if (player_step_sound != nullptr) {
        Mix_FreeChunk(player_step_sound);
        player_step_sound = nullptr;
    }
    if (player_fall_sound != nullptr) {
        Mix_FreeChunk(player_fall_sound);
        player_fall_sound = nullptr;
    }

    if (powerup_pickup_sound != nullptr) {
        Mix_FreeChunk(powerup_pickup_sound);
        powerup_pickup_sound = nullptr;
    }

    // Freeing music pointers if they are not null
    if (background_burning != nullptr) {
        Mix_FreeMusic(background_burning);
        background_burning = nullptr;
    }
    if (bgm != nullptr) {
        Mix_FreeMusic(bgm);
        bgm = nullptr;
    }
}

