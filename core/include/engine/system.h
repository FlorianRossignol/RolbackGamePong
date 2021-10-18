#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace core
{

class SystemInterface
{
public:
    //besoin de cette fonction virtuel
    virtual ~SystemInterface() = default;
    virtual void Init() = 0;
    virtual void Update(sf::Time dt) = 0;
    virtual void Destroy() = 0;
};

class OnEventInterface
{
public:
    virtual ~OnEventInterface() = default;
    virtual void OnEvent(const sf::Event& event) = 0;
};

}
