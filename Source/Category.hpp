#pragma once

// Entity/scene node category, used to dispatch commands
namespace Category {
  enum Type {
    None = 0,
    ScenePlayfieldLayer = 1 << 0,
    PlayerPaddle = 1 << 1,
    Wall = 1 << 2,
    Powerup = 1 << 3,
    HurtBall = 1 << 4,
    ParticleSystem = 1 << 5,
    SoundEffect = 1 << 6,
    Network = 1 << 7,

    Ball = Powerup | HurtBall
  };
}