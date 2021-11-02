#include <game/ball_manager.h>

namespace game
{
    BallManager::BallManager(core::EntityManager& entityManager, GameManager& gameManager,
        PhysicsManager& physicsManager) :
        ComponentManager(entityManager), gameManager_(gameManager), physicsManager_(physicsManager)
    {
    }


    void BallManager::FixedUpdate(sf::Time dt)
    {
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::BALL)))
            {
                /*auto& ball = components_[entity];
                ball.remainingTime -= dt.asSeconds();
                if (ball.remainingTime < 0.0f)
                {
                    entityManager_.DestroyEntity(entity);
                }*/
                auto ball = physicsManager_.GetBody(entity);
                //ball.velocity = ball.velocity * dt.asSeconds() + ball.position;
                
                if (ball.position.x > rectShapeDim.x /100 ||
                    ball.position.x < -rectShapeDim.x /100)
                {
                   
                    ball.velocity = core::Vec2f{ -ball.velocity.x,ball.velocity.y };
                }
                if (ball.position.y > rectShapeDim.y /100 ||
                    ball.position.y < -rectShapeDim.y /100)
                {
                    ball.velocity = core::Vec2f{ ball.velocity.x,-ball.velocity.y };
                }
                physicsManager_.SetBody(entity, ball);
                //std::cout << ball.position.x;
            }
        }
    }
}