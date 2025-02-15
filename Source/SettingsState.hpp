#pragma once

#include "State.hpp"
#include "PlayerBinding.hpp"
#include "Container.hpp"
#include "Button.hpp"
#include "Label.hpp"
#include "ScrollingPattern.hpp"

class SettingsState : public State {
public:
  SettingsState(StateStack& stack, Context context);
  ~SettingsState();

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event& event);

private:
  void updateLabels();
  void addButtonLabel(std::size_t index, std::size_t y, const std::string& text, Context context);

private:
  ScrollingPattern mPattern; //The pattern you'll see on the screen
  GUI::Container mGUIContainer;
  std::array<GUI::Button::Ptr, PlayerAction::Count>	mBindingButtons;
  std::array<GUI::Label::Ptr, PlayerAction::Count> mBindingLabels;
};