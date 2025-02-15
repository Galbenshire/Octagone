#include "pch.h"
#include "Wall.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

//Data specific to this class
namespace {
  TextureID Texture = TextureID::Entities;
  sf::IntRect TextureRect = sf::IntRect(0, 0, 30, 30);
  float Radius = static_cast<float>(TextureRect.width / 2);
}

Wall::Wall(const TextureHolder & textures)
  : Entity(textures.get(Texture), TextureRect)
{
  centerOrigin(mSprite);
}

unsigned int Wall::getCategory() const {
  return Category::Wall;
}

SceneNode::BoundingCircle Wall::getBoundingShape() const {
  return SceneNode::BoundingCircle(getWorldPosition(), Radius);
}

void Wall::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const {
  target.draw(mSprite, states);
}

void Wall::updateCurrent(sf::Time dt, CommandQueue & commands) {
  //Entity::updateCurrent(dt, commands);
}
