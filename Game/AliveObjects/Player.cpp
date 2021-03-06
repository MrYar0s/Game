#include "Player.hpp"

#include "CheckKey.hpp"

static const int kWidth = 1000;
static const int kHeight = 1000;

Player::Player(const float& dt, const float& fixdt, sf::Texture* texture) : Entity(dt, fixdt)
{
	texture_ = texture;

	InitBody();
	InitBox();
	InitAnimations();
}

Player::~Player()
{
	delete healthbar_;
}

void Player::FixedUpdate()
{
	Move();
}

void Player::Update()
{
	Animate();
	UpdateVelocity();
	KeepInBorders();
	UpdateTimer();
	if(healthbar_->GetHealthState() <= 0)
	{
		alive_ = false;
	}
}

void Player::ResolveCollision(std::vector<Platform>& platforms)
{
	sf::Vector2f pos;
	grounded_ = false;

	auto body = reinterpret_cast<sf::Sprite*>(drawable_);

	for(auto& iter : platforms)
	{
		if(!GetBox().CheckCollision(iter.GetBox(), pos))
		{
			continue;
		}

		if(pos.y < 0.f && velocity_.y >= 0.f)
		{
			grounded_ = true;
			velocity_.y = 0.f;
		}
		else if(pos.y > 0.f && velocity_.y < 0.f)
		{
			velocity_.y = 0.f;
		}

		box_.pos_ += pos;
		body->setPosition(box_.pos_);

		if(pos.x != 0.f && velocity_.y > 0.f && sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && moveable_)
		{
			bool diffY = box_.pos_.y - box_.sz_.y / 2.f > iter.GetBox().pos_.y - iter.GetBox().sz_.y;
			if(!diffY)
			{
				return;
			}
			grabbing_ = true;
			velocity_.y = 0.f;
			velocity_.x = 0.f;

			bool diffX = body->getPosition().x > iter.GetBox().pos_.x + iter.GetBox().sz_.x / 2.f;

			const int mulit = (diffX) ? -1 : 1;

			body->setScale(sf::Vector2f(2.f*mulit, 2.f));
			box_.pos_.y += -(box_.pos_.y - box_.sz_.y/2.f - iter.GetBox().pos_.y + iter.GetBox().sz_.y/2.f);
		}
		pos = sf::Vector2f();
	}
	if(velocity_.y > 0.f)
	{
		wasGrounded_ = false;
	}
}

void Player::ResolveCollision(Heart* heart)
{
	sf::Vector2f pos;
	if(GetBox().CheckCollision(heart->GetBox(), pos) && healthbar_->GetHealthState() != 3)
	{
		healthbar_->Change(false);
		heart->exist_ = false;
	}
}

bool Player::IsAlive() const
{
	return alive_;
}

void Player::Animate()
{
	if(!moveable_)
	{
		animations_["takedamage"].Update(dt_);
		if(animations_["takedamage"].GetCurrentFrame() == animations_["takedamage"].GetFrameCount() - 1)
		{
			animations_["takedamage"].SetFrame(0);
			moveable_ = true;
			accel_ = 0.85f;
		}
		return;
	}
	if(velocity_.y == 0.f)
	{
		if(velocity_.x < 0.f || velocity_.x > 0.f)
		{
			animations_["move"].Update(dt_);
		}
		else if(velocity_.x == 0.f)
		{
			animations_["idle"].Update(dt_);
		}
	}
	else
	{
		if(velocity_.y < 0)
		{
			if(animations_["jump"].GetCurrentFrame() != animations_["jump"].GetFrameCount() - 1)
			{
				animations_["jump"].Update(dt_);
			}
		}
		if(velocity_.y > 0)
		{
			animations_["fall"].Update(dt_);
		}
	}
}

void Player::KeepInBorders()
{
	if(box_.pos_.x < 0.f)
	{
		box_.pos_.x = 0.f;
	}
	else if(box_.pos_.x > kWidth)
	{
		box_.pos_.x = kWidth;
	}
}

void Player::Move()
{
	if(!grabbing_)
	{
		velocity_.y += gravity_ * fixdt_;
	}
	velocity_.x *= accel_;
	box_.pos_ += velocity_ * fixdt_;
	float val = std::abs(velocity_.x);
	velocity_.x = (val < 8.f ? 0.f : velocity_.x);

	auto body = reinterpret_cast<sf::Sprite*>(drawable_);
	body->setOrigin(sf::Vector2f(playerSize_.x/2.f, playerSize_.y/1.315f));
	body->setPosition(box_.pos_);
}

void Player::UpdateTimer()
{
	if(hit_)
	{
		if(currhit_ > 9)
		{
			currhit_ = 0;
			hit_ = false;
			return;
		}
		unsigned int blink[2] = {60, 255};

		auto body = reinterpret_cast<sf::Sprite*>(drawable_);

		sf::Color clr = body->getColor();
		body->setColor(sf::Color(clr.r, clr.g, clr.b, blink[(!(currhit_%2) ? 0 : 1)]));
		if(hitTimer_.getElapsedTime().asSeconds() > 0.175f)
		{
			currhit_++;
			hitTimer_.restart();
		}
	}
}

void Player::UpdateVelocity()
{
	if(!moveable_)
	{
		return;
	}
	if(grounded_ && sf::Keyboard::isKeyPressed(sf::Keyboard::W) && CheckKey::CheckKeyboardKey('W'))
	{
		grounded_ = false;
		grabbing_ = false;
		velocity_.y = jumpVelocity_;
		animations_["jump"].SetFrame(0);
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		velocity_.x = -moveSpeed_;
		reinterpret_cast<sf::Sprite*>(drawable_)->setScale(sf::Vector2f(-2.f, 2.f));
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		velocity_.x = moveSpeed_;
		reinterpret_cast<sf::Sprite*>(drawable_)->setScale(sf::Vector2f(2.f, 2.f));
	}
}

void Player::InitAnimations()
{
	auto sprite = reinterpret_cast<sf::Sprite*>(drawable_);

	animations_["move"].Create(8, 0, 0.08f, playerSize_, *sprite, 3);
	animations_["idle"].Create(7, 0, 0.16f, playerSize_, *sprite, 1);
	animations_["jump"].Create(2, 0, 0.125f, playerSize_, *sprite, 4);
	animations_["fall"].Create(1, 0, 0.1f, playerSize_, *sprite, 6);
	animations_["takedamage"].Create(7, 0, 0.08f, playerSize_, *sprite, 17);
}

void Player::InitBody()
{
	drawable_ = new sf::Sprite(*texture_);
	auto sprite = reinterpret_cast<sf::Sprite*>(drawable_);
	sprite->setOrigin(sf::Vector2f(playerSize_.x/2.f, playerSize_.y/1.315f));
	sprite->setScale(sf::Vector2f(2.f, 2.f));
}

void Player::InitBox()
{
	const sf::Vector2f size = sf::Vector2f(40.f, 80.f);
	box_.pos_ = sf::Vector2f(100.f, 350.f);
	box_.sz_ = size;
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(*drawable_);
	target.draw(*healthbar_);
}

void Player::ResolveCollision(std::list<Bullet>& bullets)
{
	for(auto& bullet : bullets)
	{
		sf::Vector2f pos;
		if(!GetBox().CheckCollision(bullet.GetBox(), pos) || hit_)
		{
			continue;
		}
		healthbar_->Change(true);
		hit_ = true;
		grabbing_ = false;
		moveable_ = false;
		accel_ = 0.925f;

		velocity_.y = -950.f;
		int mulit = 1;
		switch(bullet.dir_)
		{
			default:
			case RIGHT: mulit = 1;
						break;
			case LEFT: mulit = -1;
						break;
		}
		velocity_.x = mulit * 1200.f;
		reinterpret_cast<sf::Sprite*>(drawable_)->setScale(sf::Vector2f(2.f * -mulit, 2.f));
	}
}
