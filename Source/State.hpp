#pragma once
#include "StateIdentifiers.hpp"
#include "ResourceIdentifiers.hpp"

//Forward declarations
namespace sf {
  class RenderWindow;
}

class StateStack;
class MusicPlayer;
class SoundPlayer;
class PlayerBinding;

class State {
public:
  typedef std::unique_ptr<State> Ptr;

  struct Context {
    Context(sf::RenderWindow& window, TextureHolder& textures, FontHolder& fonts,
      MusicPlayer& music, SoundPlayer& sounds, PlayerBinding& keys, sf::String& ip);

    sf::RenderWindow*	window;
    TextureHolder*		textures;
    FontHolder*			fonts;
    MusicPlayer*		music;
    SoundPlayer*		sounds;
    PlayerBinding*		keys;
    sf::String*         ip;
  };

public:
  State(StateStack& stack, Context context);
  virtual ~State();

  virtual void draw() = 0;
  virtual bool update(sf::Time dt) = 0;
  virtual bool handleEvent(const sf::Event& event) = 0;

  virtual void onActivate();
  virtual void onDestroy();

protected:
  void requestStackPush(StateID stateID);
  void requestStackPop();
  void requestStateClear();

  Context getContext() const;

private:
  StateStack* mStack;
  Context mContext;
};