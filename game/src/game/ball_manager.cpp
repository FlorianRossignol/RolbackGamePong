#include <game/ball_manager.h>
#include "game/pong_player_character.h"
#include "game/game_pong_manager.h"
namespace game
{
    BallManager::BallManager(core::EntityManager& entityManager, GameManager& gameManager,
        PhysicsManager& physicsManager,
        PlayerCharacterManager& playerCharacterManager) :
        ComponentManager(entityManager), gameManager_(gameManager), physicsManager_(physicsManager),
        playerCharacterManager_(playerCharacterManager)
    {
    }


    void BallManager::FixedUpdate(sf::Time dt)
    {
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::BALL)))
            {
                
               auto ball = physicsManager_.GetBody(entity);
                
               //if the player lost ball in right the first player lose hp 
                if (ball.position.x > rectShapeDim.x / 100)
                {
                    auto firstPlayerEntity = gameManager_.GetEntityFromPlayerNumber(0);
                    ball.position = core::Vec2f{0,0};
                    auto player = playerCharacterManager_.GetComponent(firstPlayerEntity);
                    player.health--;
                    playerCharacterManager_.SetComponent(firstPlayerEntity,player);
                }
                if (ball.position.x < -rectShapeDim.x / 100)
                {
                   
                    ball.position = core::Vec2f{ 0,0 };
                   
                    auto secondPlayerEntity = gameManager_.GetEntityFromPlayerNumber(1);
                    auto player = playerCharacterManager_.GetComponent(secondPlayerEntity);
                    
                    player.health--;
                    playerCharacterManager_.SetComponent(secondPlayerEntity, player);
                }
                
                if (ball.position.y > rectShapeDim.y / 100 ||
                    ball.position.y < -rectShapeDim.y /100)
                {
                    
                    ball.velocity = core::Vec2f{ ball.velocity.x,-ball.velocity.y }; 
                    
                }
                physicsManager_.SetBody(entity, ball);
              
            }
        }
    }
}