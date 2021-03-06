#include <game/pong_rollback_manager.h>
#include <game/game_pong_manager.h>
#include <cassert>
#include <utils/log.h>
#include <fmt/format.h>

namespace game
{

    RollbackManager::RollbackManager(GameManager& gameManager, core::EntityManager& entityManager) :
        gameManager_(gameManager), entityManager_(entityManager),
        currentTransformManager_(entityManager),
        currentPhysicsManager_(entityManager), currentPlayerManager_(entityManager, currentPhysicsManager_, gameManager_),
        currentBallManager_(entityManager, gameManager,currentPhysicsManager_,currentPlayerManager_),
        lastValidatePhysicsManager_(entityManager),
        lastValidatePlayerManager_(entityManager, lastValidatePhysicsManager_, gameManager_), 
        lastValidateBallManager_(entityManager, gameManager,lastValidatePhysicsManager_,lastValidatePlayerManager_)
    {
        for (auto& input : inputs_)
        {
            std::fill(input.begin(), input.end(), 0u);
        }
        currentPhysicsManager_.RegisterTriggerListener(*this);
    }

    void RollbackManager::SimulateToCurrentFrame()
    {
        const auto currentFrame = gameManager_.GetCurrentFrame();
        const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
        //Destroying all created Entities after the last validated frame
        for (const auto& createdEntity : createdEntities_)
        {
            if (createdEntity.createdFrame > lastValidateFrame)
            {
                entityManager_.DestroyEntity(createdEntity.entity);
            }
        }
        createdEntities_.clear();
        //Remove DESTROY flags
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
            {
                entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
            }
        }
        
        //Revert the current game state to the last validated game state
        currentBallManager_.CopyAllComponents(lastValidateBallManager_.GetAllComponents());
        currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
        currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());

        for (Frame frame = lastValidateFrame + 1; frame <= currentFrame; frame++)
        {
            testedFrame_ = frame;
            //Copy player inputs to player manager
            for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
            {
                const auto playerInput = GetInputAtFrame(playerNumber, frame);
                const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
                if(playerEntity == core::EntityManager::INVALID_ENTITY)
                {
                    core::LogWarning(fmt::format("Invalid Entity in {}:line {}", __FILE__, __LINE__));
                    continue;
                }
                auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
                playerCharacter.input = playerInput;
                currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
            }
            //Simulate one frame of the game
            currentBallManager_.FixedUpdate(sf::seconds(GameManager::FixedPeriod));
            currentPlayerManager_.FixedUpdate(sf::seconds(GameManager::FixedPeriod));
            currentPhysicsManager_.FixedUpdate(sf::seconds(GameManager::FixedPeriod));
        }
        //Copy the physics states to the transforms
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            if (!entityManager_.HasComponent(entity,
                                             static_cast<core::EntityMask>(core::ComponentType::BODY2D) |
                                             static_cast<core::EntityMask>(core::ComponentType::TRANSFORM)))
                continue;
            const auto& body = currentPhysicsManager_.GetBody(entity);
            currentTransformManager_.SetPosition(entity, body.position);
            
        }
    }
    void RollbackManager::SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame)
    {
        //Should only be called on the server
        if (currentFrame_ < inputFrame)
        {
            StartNewFrame(inputFrame);
        }
        inputs_[playerNumber][currentFrame_ - inputFrame] = playerInput;
        if (lastReceivedFrame_[playerNumber] < inputFrame)
        {
            lastReceivedFrame_[playerNumber] = inputFrame;
            //Repeat the same inputs until currentFrame
            for (size_t i = 0; i < currentFrame_ - inputFrame; i++)
            {
                inputs_[playerNumber][i] = playerInput;
            }
        }
    }

    void RollbackManager::StartNewFrame(Frame newFrame)
    {
        if (currentFrame_ > newFrame)
            return;
        const auto delta = newFrame - currentFrame_;
        if (delta == 0)
        {
            return;
        }
        for (auto& inputs : inputs_)
        {
            for (auto i = inputs.size() - 1; i >= delta; i--)
            {
                inputs[i] = inputs[i - delta];
            }

            for (Frame i = 0; i < delta; i++)
            {
                inputs[i] = inputs[delta];
            }
        }
        currentFrame_ = newFrame;
    }

    void RollbackManager::ValidateFrame(Frame newValidateFrame)
    {
        const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
        //Destroying all created Entities after the last validated frame
        for (const auto& createdEntity : createdEntities_)
        {
            if (createdEntity.createdFrame > lastValidateFrame)
            {
                entityManager_.DestroyEntity(createdEntity.entity);
            }
        }
        createdEntities_.clear();
        //Remove DESTROYED flag
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
            {
                entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
            }

        }
        createdEntities_.clear();
        //We check that we got all the inputs
        for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            if (GetLastReceivedFrame(playerNumber) < newValidateFrame)
            {
                assert(false && "We should not validate a frame if we did not receive all inputs!!!");
                return;
            }
        }
        //We use the current game state as the temporary new validate game state
        currentBallManager_.CopyAllComponents(lastValidateBallManager_.GetAllComponents());
        currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
        currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());

        //We simulate the frames until the new validated frame
        for (Frame frame = lastValidateFrame_ + 1; frame <= newValidateFrame; frame++)
        {
            testedFrame_ = frame;
            //Copy the players inputs into the player manager
            for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
            {
                const auto playerInput = GetInputAtFrame(playerNumber, frame);
                const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
                auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
                playerCharacter.input = playerInput;
                currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
            }
            //We simulate one frame
            currentBallManager_.FixedUpdate(sf::seconds(GameManager::FixedPeriod));
            currentPlayerManager_.FixedUpdate(sf::seconds(GameManager::FixedPeriod));
            currentPhysicsManager_.FixedUpdate(sf::seconds(GameManager::FixedPeriod));
        }
        //Definitely remove DESTROY entities
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
            {
                entityManager_.DestroyEntity(entity);
            }
        }
        //Copy back the new validate game state to the last validated game state
        lastValidateBallManager_.CopyAllComponents(currentBallManager_.GetAllComponents());
        
        lastValidatePlayerManager_.CopyAllComponents(currentPlayerManager_.GetAllComponents());
        lastValidatePhysicsManager_.CopyAllComponents(currentPhysicsManager_);
        lastValidateFrame_ = newValidateFrame;
        createdEntities_.clear();
    }
    void RollbackManager::ConfirmFrame(Frame newValidateFrame, const std::array<PhysicsState, maxPlayerNmb>& serverPhysicsState)
    {
        ValidateFrame(newValidateFrame);
        for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            const PhysicsState lastPhysicsState = GetValidatePhysicsState(playerNumber);
            if (serverPhysicsState[playerNumber] != lastPhysicsState)
            {
                assert(false && "Physics State are not equal");
            }
        }
    }
    PhysicsState RollbackManager::GetValidatePhysicsState(PlayerNumber playerNumber) const
    {
        PhysicsState state = 0;
        const core::Entity playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
       
        const auto& playerBody = lastValidatePhysicsManager_.GetBody(playerEntity);
     

        const auto pos = playerBody.position;
       
        const auto* posPtr = reinterpret_cast<const PhysicsState*>(&pos);
        //Adding position
        for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
        {
            state += posPtr[i];
        }

        //Adding velocity
        const auto velocity = playerBody.velocity;
        const auto* velocityPtr = reinterpret_cast<const PhysicsState*>(&velocity);
        for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
        {
            state += velocityPtr[i];
        }
        //Adding rotation
       
        for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
        {
            //state += anglePtr[i];
        }
        //Adding angular Velocity
        
        for (size_t i = 0; i < sizeof(float) / sizeof(PhysicsState); i++)
        {
           
        }
        return state;
    }

    void RollbackManager::SpawnPlayer(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::degree_t rotation)
    {
        Body playerBody;
        playerBody.position = position;
        
        
        Box playerBox;
        PlayerCharacter playerChara;
        playerBox.extends = core::Vec2f{ 10,18 } / core::pixelPerMeter * playerChara.playerScale;
        

        PlayerCharacter playerCharacter;
        playerCharacter.playerNumber = playerNumber;

        currentPlayerManager_.AddComponent(entity);
        currentPlayerManager_.SetComponent(entity, playerCharacter);
        

        currentPhysicsManager_.AddBody(entity);
        currentPhysicsManager_.SetBody(entity, playerBody);
        currentPhysicsManager_.AddBox(entity);
        currentPhysicsManager_.SetBox(entity, playerBox);
        

        lastValidatePlayerManager_.AddComponent(entity);
        lastValidatePlayerManager_.SetComponent(entity, playerCharacter);

        lastValidatePhysicsManager_.AddBody(entity);
        lastValidatePhysicsManager_.SetBody(entity, playerBody);
        lastValidatePhysicsManager_.AddBox(entity);
        lastValidatePhysicsManager_.SetBox(entity, playerBox);

        currentTransformManager_.AddComponent(entity);
        currentTransformManager_.SetPosition(entity, position);
        currentTransformManager_.SetRotation(entity, rotation);
        currentTransformManager_.SetScale(entity, core::Vec2f{ 5,5 });
    }

    PlayerInput RollbackManager::GetInputAtFrame(PlayerNumber playerNumber, Frame frame)
    {
        assert(currentFrame_ - frame < inputs_[playerNumber].size() &&
            "Trying to get input too far in the past");
        return inputs_[playerNumber][currentFrame_ - frame];
    }

    void RollbackManager::OnTrigger(core::Entity entity1, core::Entity entity2)
    {
        std::function<void(const PlayerCharacter&, core::Entity, const Ball&, core::Entity)> ManageCollision =
            [this](const auto& player, auto playerEntity, const auto& ball, auto ballEntity)
        {
            auto ballbody = currentPhysicsManager_.GetBody(ballEntity);
            if (player.playerNumber %2 == 0)
            {
                ballbody.velocity = core::Vec2f{ -abs(ballbody.velocity.x),ballbody.velocity.y };
            }
            else
            {
                ballbody.velocity = core::Vec2f{ abs(ballbody.velocity.x),ballbody.velocity.y };
            }
            currentPhysicsManager_.SetBody(ballEntity, ballbody);
           
            {
                
                
            }
           
            
                
        };

        if (entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
            entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BALL)))
        {
            const auto& player = currentPlayerManager_.GetComponent(entity1);
            const auto& bullet = currentBallManager_.GetComponent(entity2);
            ManageCollision(player, entity1, bullet, entity2);

        }
        if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
            entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BALL)))
        {
            const auto& player = currentPlayerManager_.GetComponent(entity2);
            const auto& bullet = currentBallManager_.GetComponent(entity1);
            ManageCollision(player, entity2, bullet, entity1);
        }
        
        
    }

    void RollbackManager::SpawnBalle(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Vec2f velocity)
    {
        Ball ball;
        Box ballBox;
        Body ballBody;
        
        ballBody.position = position;
        
        createdEntities_.push_back({ entity, testedFrame_ });
        ballBox.extends = core::Vec2f{ 16,16 } / core::pixelPerMeter / 2;
        ballBody.velocity = core::Vec2f{ 1,1 };

        
        currentBallManager_.AddComponent(entity);
        currentBallManager_.SetComponent(entity, ball);

        currentPhysicsManager_.AddBody(entity);
        currentPhysicsManager_.SetBody(entity, ballBody);
        currentPhysicsManager_.AddBox(entity);
        currentPhysicsManager_.SetBox(entity, ballBox);

        lastValidateBallManager_.AddComponent(entity);
        lastValidateBallManager_.SetComponent(entity, ball);
 
        lastValidatePhysicsManager_.AddBody(entity);
        lastValidatePhysicsManager_.SetBody(entity, ballBody);
        lastValidatePhysicsManager_.AddBox(entity);
        lastValidatePhysicsManager_.SetBox(entity, ballBox);


        currentTransformManager_.AddComponent(entity);
        currentTransformManager_.SetPosition(entity, position);
        currentTransformManager_.SetScale(entity, ballNewScale * ballScale);
        currentTransformManager_.SetRotation(entity, core::degree_t(0.0f));
        
    }

    void RollbackManager::DestroyEntity(core::Entity entity)
    {
        
    }
}
