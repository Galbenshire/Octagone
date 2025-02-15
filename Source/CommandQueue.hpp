#pragma once
#include "Command.hpp"

class CommandQueue {
public:
public:
  void push(const Command& command);
  Command pop();
  bool isEmpty() const;

private:
  std::queue<Command> mQueue;
};