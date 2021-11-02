#pragma once
#include <SFML/System/Time.hpp>
#include <maths/vec2.h>
#include "game_pong_globals.h"
#include "physics_pong_manager.h"

//class pour gérer la boule du pong 
namespace game
{
    // structure de la boule 
    struct Ball
    {
        core::Vec2f velocity = { 0.0f,0.0f };
        core::Vec2f position = { 0.0f,0.0f };
        float R = 2.0f;
        //float remainingTime = 0.0f;
        //référence au nombre impossible de player
        PlayerNumber playerNumber = INVALID_PLAYER;
    };

    class GameManager;
    class BallManager : public core::ComponentManager<Ball, static_cast<core::EntityMask>(ComponentType::BALL)>
    {
    public:
        explicit BallManager(core::EntityManager& entityManager, GameManager& gameManager,PhysicsManager& physicsManager);
        void FixedUpdate(sf::Time dt);
    private:
        GameManager& gameManager_;
        PhysicsManager& physicsManager_;
    };
}
