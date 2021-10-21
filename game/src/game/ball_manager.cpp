#include <game/ball_manager.h>

namespace game
{
    BallManager::BallManager(core::EntityManager& entityManager, GameManager& gameManager) :
        ComponentManager(entityManager), gameManager_(gameManager)
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
                auto& ball = components_[entity];
                //ball.velocity = ball.velocity;
                //ball.position = ball.velocity;
                ball.velocity = ball.velocity * dt.asSeconds() + ball.position;
                
            }
        }
    }
}