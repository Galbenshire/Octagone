#pragma once
//A list of all the actions a player can possibly take
//Not using enum class, since we'll want a count of available actions
namespace PlayerAction {
  enum Type {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Count
  };
}

//This class bridges the gap between an game action and its corresponding inputs
//It takes in an input and, if a valid input, returns an action
//Supports the keyboard and the input axises of a gamepad (i.e. Directional Pad or Left Stick)
class PlayerBinding {
public:
  typedef PlayerAction::Type Action;

private:
  typedef std::vector<std::string> StringVector;

  //Since a joystick axis goes in two directions (- & +), it's going to take in two sperate actions
  struct AxisAction {
    //A default constructor is necessary so that '=' assignments with mAxisMap will work
	AxisAction()
	{}

	AxisAction(Action positiveAction, Action negativeAction)
		: positiveAction(positiveAction)
		, negativeAction(negativeAction)
	{}

	Action positiveAction;
	Action negativeAction;
  };

public:
  explicit PlayerBinding();

  void assignKey(Action action, sf::Keyboard::Key key);
  sf::Keyboard::Key getAssignedKey(Action action) const;

  bool checkAction(sf::Keyboard::Key key, Action& out) const;
  bool checkAction(sf::Joystick::Axis axis, float position, Action& out, Action& outOpposite) const;
  std::vector<Action> getRealtimeActions() const;

  void resetBindings();

  static bool isRealtimeAction(PlayerAction::Type action);

private:
  void initializeDefaultActions(); //These are what the bindings will be by default
  void initializeActionsInFile(); //If a binding file exists, use it to assign actions

private:
  std::unordered_map<sf::Keyboard::Key, Action> mKeyMap; //Actions will occur if a valid key is pressed
  std::unordered_map<sf::Joystick::Axis, AxisAction> mAxisMap; //Actions will occur if a valid axis is moved
};