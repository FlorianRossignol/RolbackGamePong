#pragma once
#include <SFML/System/Time.hpp>

#include "game_pong_globals.h"

namespace game
{
    class PhysicsManager;

    struct PlayerCharacter
    {
        
        PlayerInput input = 0u;
        PlayerNumber playerNumber = INVALID_PLAYER;
        short health = playerHealth;
        float invincibilityTime = 0.0f;
        static constexpr float playerScale = 5.0f;
        static constexpr float playerHalfScale = 0.5f;
    };
    class GameManager;
    class PlayerCharacterManager : public core::ComponentManager<PlayerCharacter, core::EntityMask(ComponentType::PLAYER_CHARACTER)>
    {
    public:
        explicit PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
        void FixedUpdate(sf::Time dt);

    private:
        PhysicsManager& physicsManager_;
        GameManager& gameManager_;
    };
}
