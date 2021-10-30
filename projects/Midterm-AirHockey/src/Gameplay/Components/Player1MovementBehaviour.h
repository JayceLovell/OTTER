#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

/// <summary>
/// hopefully this moves the paddle
/// </summary>
class Player1MovementBehaviour :public Gameplay::IComponent
{
public:
	typedef std::shared_ptr<Player1MovementBehaviour> Sptr;

	Player1MovementBehaviour();
	virtual ~Player1MovementBehaviour();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(Player1MovementBehaviour);
	virtual nlohmann::json ToJson() const override;
	static Player1MovementBehaviour::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _speed;
	float _posX;
	float _posY;

	Gameplay::Physics::RigidBody::Sptr _body;
};

