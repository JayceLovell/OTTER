#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

/// <summary>
/// hopefully this moves the paddle
/// </summary>
class Player2MovementBehaviour :public Gameplay::IComponent
{
public:
	typedef std::shared_ptr<Player2MovementBehaviour> Sptr;

	Player2MovementBehaviour();
	virtual ~Player2MovementBehaviour();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(Player2MovementBehaviour);
	virtual nlohmann::json ToJson() const override;
	static Player2MovementBehaviour::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _speed;
	float _posX;
	float _posY;

	Gameplay::Physics::RigidBody::Sptr _body;
};

