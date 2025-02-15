#include "pch.h"
#include "SettingsState.hpp"
#include "Utility.hpp"
#include "Constants.hpp"
#include "ResourceHolder.hpp"
#include "SoundPlayer.hpp"

//Data specific to this class
namespace {
  const float ScrollSpeed = 0.5f;
}

SettingsState::SettingsState(StateStack & stack, Context context)
  : State(stack, context)
  , mPattern(context.textures->get(TextureID::TitleScreen), sf::Vector2f(0.f, ScrollSpeed))
  , mGUIContainer()
{
  mPattern.setColor(getGameColor(GameColors::COLOR_ONE), ScrollingPattern::Pattern);

  addButtonLabel(PlayerAction::MoveLeft, 0, "Move Left", context);
  addButtonLabel(PlayerAction::MoveRight, 1, "Move Right", context);
  addButtonLabel(PlayerAction::MoveUp, 2, "Move Up", context);
  addButtonLabel(PlayerAction::MoveDown, 3, "Move Down", context);

  updateLabels();

  auto defaultsButton = std::make_shared<GUI::Button>(context);
  defaultsButton->setPosition(144.f, 72.f*PlayerAction::Count + 16.f);
  defaultsButton->setText("DEFAULTS");
  defaultsButton->setCallback([this]() {
    getContext().keys->resetBindings();
    getContext().sounds->play(SFXID::ResetBindings);
    updateLabels();
  });

  auto backButton = std::make_shared<GUI::Button>(context);
  backButton->setPosition(48.f, 384.f);
  backButton->setText("BACK");
  backButton->setCallback(std::bind(&SettingsState::requestStackPop, this));

  mGUIContainer.pack(defaultsButton);
  mGUIContainer.pack(backButton);
}

SettingsState::~SettingsState() {
  //Write the settings to a file, for PlayerBinding to use next time the application is opened
  std::ofstream outputFile;
  outputFile.open(BINDINGS_FILENAME);

  for (std::size_t i = 0; i < PlayerAction::Count; ++i) {
    auto action = static_cast<PlayerAction::Type>(i);

    sf::Keyboard::Key key = getContext().keys->getAssignedKey(action);
    outputFile << action << ":" << key << std::endl;
  }
  
  outputFile.close();
}

void SettingsState::draw() {
  sf::RenderWindow& window = *getContext().window;
  window.setView(window.getDefaultView());

  window.draw(mPattern);
  window.draw(mGUIContainer);
}

bool SettingsState::update(sf::Time dt) {
  mPattern.update();
  return false;
}

bool SettingsState::handleEvent(const sf::Event & event) {
  bool isKeyBinding = false;

  // Iterate through all key binding buttons to see if they are being pressed, waiting for the user to enter a key
  for (std::size_t i = 0; i < PlayerAction::Count; ++i) {
    if (mBindingButtons[i]->isActive()) {
      isKeyBinding = true;
      if (event.type == sf::Event::KeyReleased) {
          getContext().keys->assignKey(static_cast<PlayerAction::Type>(i), event.key.code);

        mBindingButtons[i]->deactivate();
      }
      break;
    }
  }

  // If pressed button changed key bindings, update labels; otherwise consider other buttons in container
  if (isKeyBinding)
    updateLabels();
  else
    mGUIContainer.handleEvent(event);

  return false;
}

void SettingsState::updateLabels() {
  for (std::size_t i = 0; i < PlayerAction::Count; ++i) {
    auto action = static_cast<PlayerAction::Type>(i);

    // Get keys of both players
    sf::Keyboard::Key key = getContext().keys->getAssignedKey(action);

    // Assign both key strings to labels
    mBindingLabels[i]->setText(toString(key));
  }
}

void SettingsState::addButtonLabel(std::size_t index, std::size_t y, const std::string & text, Context context) {
  mBindingButtons[index] = std::make_shared<GUI::Button>(context);
  mBindingButtons[index]->setPosition(144.f, 72.f*y + 16.f);
  mBindingButtons[index]->setText(text);
  mBindingButtons[index]->setToggle(true);

  mBindingLabels[index] = std::make_shared<GUI::Label>("", *context.fonts);
  mBindingLabels[index]->setPosition(312.f, 72.f*y + 32.f);

  mGUIContainer.pack(mBindingButtons[index]);
  mGUIContainer.pack(mBindingLabels[index]);
}
