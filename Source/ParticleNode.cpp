#include "pch.h"
#include "ParticleNode.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

//Particle Data
namespace {
  std::vector<Particle> initializePaddleSpawnData() {
    std::vector<Particle> data(Particle::ParticleCount);

    data[Particle::BallTrail].startColor = getGameColor(GameColors::COLOR_FOUR);
    data[Particle::BallTrail].endColor = sf::Color::White;
    data[Particle::BallTrail].lifetime = sf::seconds(0.75f);

    return data;
  }

  const std::vector<Particle> ParticleTable = initializePaddleSpawnData();
}

ParticleNode::ParticleNode(Particle::Type type, const TextureHolder & textures)
  : SceneNode()
  , mParticles()
  , mTexture(textures.get(TextureID::Particle))
  , mType(type)
  , mVertexArray(sf::Quads)
  , mNeedsVertexUpdate(true)
{
}

void ParticleNode::addParticle(sf::Vector2f position) {
  Particle particle;
  particle.position = position;
  particle.startColor = ParticleTable[mType].startColor;
  particle.endColor = ParticleTable[mType].endColor;
  particle.lifetime = ParticleTable[mType].lifetime;

  mParticles.push_back(particle);
}

Particle::Type ParticleNode::getParticleType() const {
  return mType;
}

unsigned int ParticleNode::getCategory() const {
  return Category::ParticleSystem;
}

void ParticleNode::updateCurrent(sf::Time dt, CommandQueue & commands) {
  // Remove expired particles at beginning
  while (!mParticles.empty() && mParticles.front().lifetime <= sf::Time::Zero)
    mParticles.pop_front();

  // Decrease lifetime of existing particles
  for (Particle& particle : mParticles)
    particle.lifetime -= dt;

  mNeedsVertexUpdate = true;
}

void ParticleNode::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const {
  if (mNeedsVertexUpdate) {
    computeVertices();
    mNeedsVertexUpdate = false;
  }

  // Apply particle texture
  states.texture = &mTexture;

  // Draw vertices
  target.draw(mVertexArray, states);
}

void ParticleNode::addVertex(float worldX, float worldY, float texCoordX, float texCoordY, const sf::Color & color) const {
  sf::Vertex vertex;
  vertex.position = sf::Vector2f(worldX, worldY);
  vertex.texCoords = sf::Vector2f(texCoordX, texCoordY);
  vertex.color = color;

  mVertexArray.append(vertex);
}

void ParticleNode::computeVertices() const {
  sf::Vector2f size(mTexture.getSize());
  sf::Vector2f half = size / 2.f;

  // Refill vertex array
  mVertexArray.clear();
  for (const Particle& particle : mParticles) {
    sf::Vector2f pos = particle.position;

    float ratio = particle.lifetime.asSeconds() / ParticleTable[mType].lifetime.asSeconds();
    sf::Color color = lerpColor(particle.endColor, particle.startColor, ratio);
    color.a = static_cast<sf::Uint8>(255 * std::max(ratio, 0.f));

    addVertex(pos.x - half.x, pos.y - half.y, 0.f, 0.f, color);
    addVertex(pos.x + half.x, pos.y - half.y, size.x, 0.f, color);
    addVertex(pos.x + half.x, pos.y + half.y, size.x, size.y, color);
    addVertex(pos.x - half.x, pos.y + half.y, 0.f, size.y, color);
  }
}
