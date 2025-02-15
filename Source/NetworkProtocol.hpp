#pragma once

const unsigned short ServerPort = 50025;

// The follow enums have been given lower integer types (i.e short/char)
// To decrease the amount of data sent over packets

// Packets originated in the server
enum class ServerPacket : sf::Uint8 {
  BroadcastMessage,	// format: [Int32:packetType] [string:message]
  SpawnSelf,			// format: [Int32:packetType]
  InitialState,
  StartGame,
  SpawnBall,
  RemoveBalls,
  RemoveDeadPlayer,
  PlayerEvent,
  PlayerRealtimeChange,
  PlayerConnect,
  PlayerDisconnect,
  UpdateClientState,
  EndGame
};

// Packets originated in the client
enum class ClientPacket : sf::Uint8 {
  StartGame,
  PlayerEvent,
  PlayerRealtimeChange,
  PlayerRealtimeChangeGamepad,
  PositionUpdate,
  GameEvent,
  Quit
};

// The types of actions a player can partake in
namespace PlayerActions {
  enum Action {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    ActionCount
  };
}

namespace GameActions {
  enum Type : sf::Uint8 {
    PlayerHurt
  };

  struct Action {
    Action() {} //Leave uninitialized

    Action(Type type, sf::Vector2f position)
      : type(type)
      , position(position)
    {}

    Type type;
    sf::Vector2f position;
  };
}