#include "pch.h"
#include "SceneNode.hpp"
#include "Command.hpp"
#include "Utility.hpp"

SceneNode::SceneNode(Category::Type category)
  : mChildren()
  , mParent(nullptr)
  , mDefaultCategory(category)
{
}

void SceneNode::attachChild(Ptr child) {
  child->mParent = this;
  mChildren.push_back(std::move(child));
}

SceneNode::Ptr SceneNode::detachChild(const SceneNode & node) {
  auto found = std::find_if(mChildren.begin(), mChildren.end(), [&](Ptr& p) { return p.get() == &node; });
  assert(found != mChildren.end());

  Ptr result = std::move(*found);
  result->mParent = nullptr;
  mChildren.erase(found);
  return result;
}

void SceneNode::update(sf::Time dt, CommandQueue & commands) {
  updateCurrent(dt, commands);
  updateChildren(dt, commands);
}

sf::Vector2f SceneNode::getWorldPosition() const {
  return getWorldTransform() * sf::Vector2f();
}

sf::Transform SceneNode::getWorldTransform() const {
  sf::Transform transform = sf::Transform::Identity;

  for (const SceneNode* node = this; node != nullptr; node = node->mParent)
    transform = node->getTransform() * transform;

  return transform;
}

void SceneNode::onCommand(const Command & command, sf::Time dt) {
  // Command current node, if category matches
  if (command.category & getCategory())
    command.action(*this, dt);

  // Command children
  for (Ptr& child : mChildren)
    child->onCommand(command, dt);
}

unsigned int SceneNode::getCategory() const {
  return mDefaultCategory;
}

void SceneNode::checkSceneCollision(SceneNode & sceneGraph, std::set<Pair>& collisionPairs) {
  checkNodeCollision(sceneGraph, collisionPairs);

  for (Ptr& child : sceneGraph.mChildren)
    checkSceneCollision(*child, collisionPairs);
}

void SceneNode::checkNodeCollision(SceneNode & node, std::set<Pair>& collisionPairs) {
  using namespace NodeUtil;

  if (this != &node && collision(*this, node) && !isDestroyed() && !node.isDestroyed())
    collisionPairs.insert(std::minmax(this, &node));

  for (Ptr& child : mChildren)
    child->checkNodeCollision(node, collisionPairs);
}

void SceneNode::removeWrecks() {
  // Remove all children which request so
  auto wreckfieldBegin = std::remove_if(mChildren.begin(), mChildren.end(), std::mem_fn(&SceneNode::isMarkedForRemoval));
  mChildren.erase(wreckfieldBegin, mChildren.end());

  // Call function recursively for all remaining children
  std::for_each(mChildren.begin(), mChildren.end(), std::mem_fn(&SceneNode::removeWrecks));
}

sf::FloatRect SceneNode::getBoundingRect() const {
  return sf::FloatRect();
}

SceneNode::BoundingCircle SceneNode::getBoundingShape() const {
  return BoundingCircle();
}

bool SceneNode::isMarkedForRemoval() const {
  // By default, remove node if entity is destroyed
  return isDestroyed();
}

bool SceneNode::isDestroyed() const {
  // By default, scene node needn't be removed
  return false;
}

void SceneNode::updateCurrent(sf::Time dt, CommandQueue & commands) {
  //Sorry nothing
}

void SceneNode::updateChildren(sf::Time dt, CommandQueue & commands) {
  for (Ptr& child : mChildren)
    child->update(dt, commands);
}

void SceneNode::draw(sf::RenderTarget & target, sf::RenderStates states) const {
  // Apply transform of current node
  states.transform *= getTransform();

  // Draw node and children with changed transform
  drawCurrent(target, states);
  drawChildren(target, states);

  // Draw bounding rectangle - disabled by default
  //drawBoundingShape(target, states);
}

void SceneNode::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const {
  //Sorry nothing
}

void SceneNode::drawChildren(sf::RenderTarget & target, sf::RenderStates states) const {
  for (const Ptr& child : mChildren)
    child->draw(target, states);
}

void SceneNode::drawBoundingShape(sf::RenderTarget & target, sf::RenderStates states) const {
  BoundingCircle circle = getBoundingShape();
  if (circle.radius <= 0.f) { return; } //How would you draw a circle that has no radius?

  sf::CircleShape shape;
  shape.setRadius(circle.radius);
  centerOrigin(shape);
  shape.setPosition(circle.position);
  shape.setFillColor(sf::Color::Transparent);
  shape.setOutlineColor(sf::Color::Green);
  shape.setOutlineThickness(1.f);

  target.draw(shape);
}

bool NodeUtil::collision(const SceneNode & lhs, const SceneNode & rhs) {
  //Credit to this page for the detection algorithm
  //https://stackoverflow.com/questions/1736734/circle-circle-collision
  //Accessed 25/01/19 02:52PM
  SceneNode::BoundingCircle lhCircle = lhs.getBoundingShape();
  SceneNode::BoundingCircle rhCircle = rhs.getBoundingShape();

  return std::pow(lhCircle.position.x - rhCircle.position.x,2)
    + std::pow(lhCircle.position.y - rhCircle.position.y, 2)
    <= std::pow(lhCircle.radius + rhCircle.radius, 2);
}

float NodeUtil::distance(const SceneNode & lhs, const SceneNode & rhs) {
  return length(lhs.getWorldPosition() - rhs.getWorldPosition());
}
