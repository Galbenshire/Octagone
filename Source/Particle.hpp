#pragma once
struct Particle {
  enum Type {
    BallTrail,
    ParticleCount
  };

  sf::Vector2f position;
  sf::Color startColor;
  sf::Color endColor;
  sf::Time lifetime;
};