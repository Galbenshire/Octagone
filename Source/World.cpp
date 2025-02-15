#include "pch.h"
#include "World.hpp"
#include "SpriteNode.hpp"
#include "Wall.hpp"
#include "Ball.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"
#include "Utility.hpp"
#include "Constants.hpp"

//Functions that don't neccessarily need to be methods of the 'World' class but won't be used in any other class
//This is something the SFML source code does for some classes
//Example: https://github.com/SFML/SFML/blob/master/src/SFML/Graphics/Text.cpp
namespace {
  bool matchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2) {
    unsigned int category1 = colliders.first->getCategory();
    unsigned int category2 = colliders.second->getCategory();

    // Make sure first pair entry has category type1 and second has type2
    if (type1 & category1 && type2 & category2) {
      return true;
    } else if (type1 & category2 && type2 & category1) {
      std::swap(colliders.first, colliders.second);
      return true;
    } else {
      return false;
    }
  }
}

World::World(sf::RenderTarget & outputTarget, TextureHolder & textures, FontHolder & fonts, SoundPlayer & sounds)
  : mTarget(outputTarget)
  , mSceneTexture()
  , mWorldView(outputTarget.getDefaultView())
  , mTextures(textures)
  , mFonts(fonts)
  , mSounds(sounds)
  , mSceneGraph()
  , mSceneLayers()
  , mWorldBounds(sf::Vector2f(), mWorldView.getSize())
  , mNetworkNode(nullptr)
{
  mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

  buildScene();

  // Prepare the view
  mWorldView.setCenter(VIEW_WIDTH / 2, VIEW_HEIGHT / 2);
  mSounds.setListenerPosition(mWorldView.getCenter());
}

void World::update(sf::Time dt) {
  for (Paddle* p : mPlayerPaddles)
    p->setVelocity(0.f, 0.f);

  // Setup commands to destroy entities outside view
  destroyEntitiesOutsideView();

  // Forward commands to scene graph
  while (!mCommandQueue.isEmpty())
    mSceneGraph.onCommand(mCommandQueue.pop(), dt);

  // Collision detection and response
  handleCollisions();

  // Remove all destroyed entities
  mSceneGraph.removeWrecks();

  // Regular update step, adapt position (correct if outside view)
  mSceneGraph.update(dt, mCommandQueue);

  mSounds.removeStoppedSounds(); //Remove unused sounds
}

void World::draw() {
  if (PostEffect::isSupported() && mEffects.canUse) {
    mSceneTexture.clear();
    mSceneTexture.setView(mWorldView);
    mSceneTexture.draw(mSceneGraph);
    mSceneTexture.display();
    mEffects.currentEffect->apply(mSceneTexture, mTarget);
  } else {
    mTarget.setView(mWorldView);
    mTarget.draw(mSceneGraph);
  }

  return;
}

sf::FloatRect World::getWorldBounds() const {
  return mWorldBounds;
}

CommandQueue & World::getCommandQueue() {
  return mCommandQueue;
}

Paddle * World::addPaddle(sf::Int8 identifier, bool isLocalPlayer) {
  std::unique_ptr<Paddle> player(new Paddle(mTextures, identifier));

  if (isLocalPlayer)
    player->addLocalIndicator(mFonts);

  mPlayerPaddles.push_back(player.get());
  mSceneLayers[LowerPlayfield]->attachChild(std::move(player));
  return mPlayerPaddles.back();
}

void World::removePaddle(sf::Int8 identifier) {
  Paddle* paddle = getPaddle(identifier);
  if (paddle) {
	paddle->destroy();
	mPlayerPaddles.erase(std::find(mPlayerPaddles.begin(), mPlayerPaddles.end(), paddle));
  }
}

Paddle * World::getPaddle(sf::Int8 identifier) const {
  for (Paddle* p : mPlayerPaddles) {
    if (p->getIdentifier() == identifier)
      return p;
  }

  return nullptr;
}

void World::addBall(const sf::Uint8& type, const float & angle, const float & speed, const float & delay) {
  Ball::Type typeCasted = static_cast<Ball::Type>(type);
  std::unique_ptr<Ball> ball(new Ball(mTextures, typeCasted, angle, speed, delay));
  ball->setPosition(VIEW_WIDTH / 2, VIEW_HEIGHT / 2);
  mSceneLayers[UpperPlayfield]->attachChild(std::move(ball));
}

void World::removeBalls() {
  Command command;
  command.category = Category::Ball;
  command.action = derivedAction<Ball>([](Ball& e, sf::Time) {
    e.remove();
  });
  mCommandQueue.push(command);
}

bool World::pollGameAction(GameActions::Action & out) {
  return mNetworkNode->pollGameAction(out);
}

void World::setPauseEffect() {
  mEffects.canUse = true;
  mEffects.currentEffect = &mEffects.greyscaleEffect;
}

void World::setGameEndEffect() {
  mEffects.canUse = true;
  mEffects.currentEffect = &mEffects.invertEffect;
}

void World::resetEffect() {
  mEffects.canUse = false;
}

void World::handleCollisions() {
  std::set<SceneNode::Pair> collisionPairs;
  mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

  for (SceneNode::Pair pair : collisionPairs) {
    if (matchesCategories(pair, Category::Ball, Category::Wall)) {
      auto& ball = static_cast<Ball&>(*pair.first);
      auto& wall = static_cast<Wall&>(*pair.second);

      // Collision: Bounce the wall off the wall
      ball.bounceOffWall(wall, getCommandQueue());
    } else if (matchesCategories(pair, Category::HurtBall, Category::PlayerPaddle)) {
      auto& ball = static_cast<Ball&>(*pair.first);
      auto& player = static_cast<Paddle&>(*pair.second);

      // Collision: This player will lose a life
      if (ball.canCollideWithPlayer()) {
        ball.setCollidableWithPlayer(false);
        if (player.getLives() > 0) { player.loseLife(); }

        //Emit a network game action telling the server that balls should be removed
        destroyBallsInView(ball.getWorldPosition());

        sf::Vector2f worldPosition = ball.getWorldPosition();
        Command command;
        command.category = Category::SoundEffect;
        command.action = derivedAction<SoundNode>([worldPosition](SoundNode& node, sf::Time) {
          node.playSound(SFXID::Hit, worldPosition);
        });
        getCommandQueue().push(command);
      }
    } else if (matchesCategories(pair, Category::Powerup, Category::PlayerPaddle)) {
      auto& ball = static_cast<Ball&>(*pair.first);
      auto& player = static_cast<Paddle&>(*pair.second);

      // Collision: Give the player faster speed
      player.givePowerup(getCommandQueue());
      ball.destroy();
    }

  }
}

void World::buildScene() {
  // Initialize the different layers
  for (std::size_t i = 0; i < LayerCount; ++i) {
    Category::Type category = (i == UpperPlayfield) ? Category::ScenePlayfieldLayer : Category::None;

    SceneNode::Ptr layer(new SceneNode(category));
    mSceneLayers[i] = layer.get();

    mSceneGraph.attachChild(std::move(layer));
  }

  //Make the background
  //
  sf::Texture& bgTexture = mTextures.get(TextureID::Pixel);
  bgTexture.setRepeated(true);
  sf::IntRect textureRect(mWorldBounds);
  //
  std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(bgTexture, textureRect));
  backgroundSprite->getSprite().setColor(getGameColor(GameColors::COLOR_ONE));
  mSceneLayers[Background]->attachChild(std::move(backgroundSprite));

  //Add the walls around the arena
  addWalls();

  //Add 'ball trail particle node
  std::unique_ptr<ParticleNode> trailNode(new ParticleNode(Particle::BallTrail, mTextures));
  mSceneLayers[UpperPlayfield]->attachChild(std::move(trailNode));

  // Add sound effect node
  std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
  mSceneGraph.attachChild(std::move(soundNode));

  //Add network node
  std::unique_ptr<NetworkNode> networkNode(new NetworkNode());
  mNetworkNode = networkNode.get();
  mSceneGraph.attachChild(std::move(networkNode));
}

//Adds the walls that lie between player goallines
void World::addWalls() {
  //A lambda to calculate the relative angle of a wall.
  //It starts off at the left most side, rather than the right
  auto calculatedPosition = [](const float& angleInc) {
    return sf::Vector2f(216.f * cosf(toRadian(157.5f + angleInc)) + (VIEW_WIDTH / 2), 216.f * sinf(toRadian(157.5f + angleInc)) + (VIEW_HEIGHT / 2));
  };

  //Spawn the walls
  for (int i = 0; i < 8; ++i)
    addWall(calculatedPosition(45.f * i));
}

void World::addWall(const sf::Vector2f& pos) {
  std::unique_ptr<Wall> wall(new Wall(mTextures));
  wall->setPosition(pos);

  mSceneLayers[LowerPlayfield]->attachChild(std::move(wall));
}

void World::destroyBallsInView(const sf::Vector2f& position) {
  Command command;
  command.category = Category::Network;
  command.action = derivedAction<NetworkNode>([position](NetworkNode& node, sf::Time) {
    node.notifyGameAction(GameActions::PlayerHurt, position);
  });
  mCommandQueue.push(command);
}

void World::destroyEntitiesOutsideView() {
  Command command;
  command.category = Category::Ball;
  command.action = derivedAction<Entity>([this](Entity& e, sf::Time) {
    if (!getWorldBounds().intersects(e.getBoundingRect()))
      e.remove();
  });

  mCommandQueue.push(command);
}
