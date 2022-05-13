#ifndef GAME_ALIVEOBJECTS_HPP
#define GAME_ALIVEOBJECTS_HPP

#include "CollisionBox.hpp"
#include <SFML/Graphics.hpp>

class Entity : public sf::Drawable
{
public:
	Entity(float dt, float fixdt);
	virtual ~Entity();

	virtual void FixedUpdate() = 0;
	virtual void Update() = 0;

	const CollisionBox& GetBox() const;
protected:
	CollisionBox box_;
	sf::Texture* texture_ = nullptr;
	sf::Drawable* drawable_ = nullptr;

	float dt_;
	float fixdt_;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};


#endif //GAME_ALIVEOBJECTS_HPP