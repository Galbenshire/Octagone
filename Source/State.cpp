#include "pch.h"
#include "State.hpp"
#include "StateStack.hpp"

State::Context::Context(sf::RenderWindow & window, TextureHolder & textures, FontHolder & fonts, MusicPlayer & music, SoundPlayer & sounds, PlayerBinding & keys, sf::String & ip)
  : window(&window)
  , textures(&textures)
  , fonts(&fonts)
  , music(&music)
  , sounds(&sounds)
  , keys(&keys)
  , ip(&ip)
{
}

State::State(StateStack & stack, Context context)
  : mStack(&stack)
  , mContext(context)
{
}

State::~State()
{
}

void State::onActivate()
{
}

void State::onDestroy()
{
}

void State::requestStackPush(StateID stateID) {
  mStack->pushState(stateID);
}

void State::requestStackPop() {
  mStack->popState();
}

void State::requestStateClear() {
  mStack->clearStates();
}

State::Context State::getContext() const {
  return mContext;
}
