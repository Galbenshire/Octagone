#include "pch.h"
#include "IPConfigState.hpp"
#include "Constants.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

IPConfigState::IPConfigState(StateStack & stack, Context context)
	: State(stack, context)
	, mPattern(context.textures->get(TextureID::TitleScreen))
	, mGUIContainer()
	, mIPFormat("^[\\d]{1,3}.[\\d]{1,3}.[\\d]{1,3}.[\\d]{1,3}$")
	, mUpdateInputDisplay(false)
{
	mPattern.setColor(getGameColor(GameColors::COLOR_TWO), ScrollingPattern::Pattern);
	mPattern.setColor(getGameColor(GameColors::COLOR_ONE), ScrollingPattern::Background);

	auto instructionsLabel = std::make_shared<GUI::Label>("PLEASE ENTER AN IP ADDRESS\nTHIS WILL BE USED TO HOST / JOIN A GAME", *context.fonts);
	instructionsLabel->setPosition(VIEW_WIDTH * 0.1f, VIEW_HEIGHT * 0.2f);

	mInputDisplay = std::make_shared<GUI::Label>(context.ip->toAnsiString(), *context.fonts);
	mInputDisplay->setPosition(VIEW_WIDTH * 0.4f, VIEW_HEIGHT * 0.5f);

	mInputResult = std::make_shared<GUI::Label>("", *context.fonts);
	mInputResult->setPosition(VIEW_WIDTH * 0.4f, VIEW_HEIGHT * 0.75f);

	mGUIContainer.pack(instructionsLabel);
	mGUIContainer.pack(mInputDisplay);
	mGUIContainer.pack(mInputResult);
}

void IPConfigState::draw() {
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());

	window.draw(mPattern);
	window.draw(mGUIContainer);
}

bool IPConfigState::update(sf::Time dt) {
	mPattern.update();

	if (mUpdateInputDisplay) {
		mInputDisplay->setText(mInputString);

		mUpdateInputDisplay = false;
	}
	return true;
}

bool IPConfigState::handleEvent(const sf::Event & event) {
  if (event.type == sf::Event::TextEntered) {
	char unicode = static_cast<char>(event.text.unicode);
	if ((unicode >= '0' && unicode <= '9') || unicode == '.') {
	  if (mInputString.getSize() < 15u) {
		std::string newDisplay = mInputString;
		newDisplay.append(1u, unicode);
		mInputString = newDisplay;
		mUpdateInputDisplay = true;
	  }
	}
  }

  if (event.type == sf::Event::KeyPressed) {
	if (event.key.code == sf::Keyboard::Enter) {
	  //Use regex to check if the input string is an actual IP address format
	  std::smatch ipMatch;
	  std::string input = mInputString;
      if (std::regex_match(input, ipMatch, mIPFormat)) {
        *getContext().ip = mInputString;
        mInputResult->setText("--IP SET--");
      } else {
        mInputResult->setText("--INCORRECT IP FORMAT--");
      }
	  mUpdateInputDisplay = true;
    } else if (event.key.code == sf::Keyboard::Backspace) {
      if (mInputString.getSize() > 0) {
        mInputString.erase(mInputString.getSize() - 1u);
        mUpdateInputDisplay = true;
      }
    } else if (event.key.code == sf::Keyboard::Escape) {
	  requestStackPop();
	}
  }

  return false;
}
