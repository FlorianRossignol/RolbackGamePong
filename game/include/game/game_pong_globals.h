#pragma once
#include <SFML/Graphics/Color.hpp>
#include <array>

#include "engine/component.h"
#include "engine/entity.h"
#include "maths/angle.h"
#include "maths/vec2.h"


namespace game
{

    using PlayerNumber = std::uint8_t;
    const PlayerNumber INVALID_PLAYER = std::numeric_limits<PlayerNumber>::max();
    using ClientId = std::uint16_t;
    using Frame = std::uint32_t;

    const std::uint32_t maxPlayerNmb = 2;
    const short playerHealth = 5;
   
    const core::degree_t playerAngularSpeed = core::degree_t(90.0f);
    const float playerShootingPeriod = 0.3f;
    const float balloriginalspeed = 2.0f;
    const float ballScale = 0.5f;
    const core::Vec2f ballNewScale = {5,5};
    
   
    const core::Vec2f rectShapeDim = { 1000 /2,600 /2.0f};
    const float bulletPeriod = 3.0f;
    const float playerInvincibilityPeriod = 1.5f;
    const float invincibilityFlashPeriod = 0.5f;

    const std::array<sf::Color, std::max(maxPlayerNmb, 5u)> playerColors =
    {
      {
            sf::Color::Red,
            sf::Color::Blue,
            sf::Color::Yellow,
            sf::Color::Cyan,
            sf::Color::Green
        }
    };

    constexpr std::array<core::Vec2f, std::max(4u, maxPlayerNmb)> spawnPositions
    {
            core::Vec2f(1,0),
            core::Vec2f(-1,0),
            core::Vec2f(0,1),
            core::Vec2f(0,-1),
    };

    const std::array<core::degree_t, std::max(4u, maxPlayerNmb)> spawnRotations
    {
        core::degree_t(0.0f),
        core::degree_t(180.0f),
        core::degree_t(-90.0f),
        core::degree_t(90.0f)
    };

    enum class ComponentType : core::EntityMask
    {
        PLAYER_CHARACTER = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE),
        BALL = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 1u,
        ASTEROID = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 2u,
        PLAYER_INPUT = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 3u,
        DESTROYED = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 4u,
        WALL = static_cast<core::EntityMask>(core::ComponentType::OTHER_TYPE) << 5u,
    };

    using PlayerInput = std::uint8_t;

    namespace PlayerInputEnum
    {
        enum PlayerInput : std::uint8_t
        {
            NONE = 0u,
            UP = 1u << 0u,
            DOWN = 1u << 1u,
        };
    }
}
