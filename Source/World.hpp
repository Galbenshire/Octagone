#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "Paddle.hpp"
#include "SimpleEffect.hpp"
#include "SoundPlayer.hpp"
#include "CommandQueue.hpp"
#include "Command.hpp"
#include "NetworkProtocol.hpp"

// Forward declarations
namespace sf {
  class RenderTarget;
}
class NetworkNode;

class World : private sf::NonCopyable {
public:
  World(sf::RenderTarget& outputTarget, TextureHolder& textures, FontHolder& fonts, SoundPlayer& sounds);
  void update(sf::Time dt);
  void draw();

  sf::FloatRect getWorldBounds() const;
  CommandQueue& getCommandQueue();
  Paddle* addPaddle(sf::Int8 identifier, bool isLocalPlayer = false);
  void removePaddle(sf::Int8 identifier);
  Paddle* getPaddle(sf::Int8 identifier) const;

  void addBall(const sf::Uint8& type, const float& angle, const float& speed, const float& delay);
  void removeBalls();

  bool pollGameAction(GameActions::Action& out);

  void setPauseEffect();
  void setGameEndEffect();
  void resetEffect();

private:
  void handleCollisions();
  void buildScene();
  void addWalls();
  void addWall(const sf::Vector2f& pos);
  void destroyBallsInView(const sf::Vector2f& position);
  void destroyEntitiesOutsideView();

private:
  enum Layer {
    Background,
    LowerPlayfield,
    UpperPlayfield,
    LayerCount
  };

  struct EffectHolder {
    EffectHolder()
      : canUse(false)
      , currentEffect(&greyscaleEffect)
      , invertEffect(ShaderID::InvertPass)
      , greyscaleEffect(ShaderID::GreyscalePass)
    {}

    bool canUse;
    SimpleEffect* currentEffect;
    SimpleEffect invertEffect;
    SimpleEffect greyscaleEffect;
  };

private:
  sf::RenderTarget& mTarget;
  sf::RenderTexture mSceneTexture;
  sf::View mWorldView;
  TextureHolder& mTextures;
  FontHolder& mFonts;
  SoundPlayer& mSounds;

  SceneNode mSceneGraph;
  std::array<SceneNode*, LayerCount> mSceneLayers;
  CommandQueue mCommandQueue;

  sf::FloatRect mWorldBounds;
  std::vector<Paddle*> mPlayerPaddles;

  EffectHolder mEffects;

  NetworkNode* mNetworkNode;
};