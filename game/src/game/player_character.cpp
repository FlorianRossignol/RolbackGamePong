#include <game/pong_player_character.h>
#include <game/game_pong_manager.h>

namespace game
{
    PlayerCharacterManager::PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager) :
        ComponentManager(entityManager),
        physicsManager_(physicsManager),
        gameManager_(gameManager)

    {

    }

    void PlayerCharacterManager::FixedUpdate(sf::Time dt)
    {
        for (core::Entity playerEntity = 0; playerEntity < entityManager_.GetEntitiesSize(); playerEntity++)
        {
            if (!entityManager_.HasComponent(playerEntity,
                                                   static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
                continue;
            auto playerBody = physicsManager_.GetBody(playerEntity);
            auto playerCharacter = GetComponent(playerEntity);
            const auto input = playerCharacter.input;
            const bool up = input & PlayerInputEnum::PlayerInput::UP;
            const bool down = input & PlayerInputEnum::PlayerInput::DOWN;

            auto dir = core::Vec2f::up();
            //dir = dir.Rotate(-(playerBody.rotation + playerBody.angularVelocity * dt.asSeconds()));

            const auto acceleration = ((down ? -1.0f : 0.0f) + (up ? 1.0f : 0.0f)) *dir;
            

            playerBody.velocity += acceleration * dt.asSeconds();
            if ((playerBody.position.y > rectShapeDim.y / 100 &&
                playerBody.velocity.y > 0)
                || ( playerBody.velocity.y < 0 && playerBody.position.y < -rectShapeDim.y /100))
            {
                playerBody.velocity = core::Vec2f{ 0,0 };
            }
            physicsManager_.SetBody(playerEntity, playerBody);

            if (playerCharacter.invincibilityTime > 0.0f)
            {
                playerCharacter.invincibilityTime -= dt.asSeconds();
                SetComponent(playerEntity, playerCharacter);
            }
         
        }
    }
}
