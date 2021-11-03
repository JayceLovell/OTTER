#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Gameplay/Physics/RigidBody.h"
/// <summary>
/// Puck Class
/// </summary>
class PuckBehaviour :public Gameplay::IComponent
{
public:
	typedef std::shared_ptr<PuckBehaviour> Sptr;

	PuckBehaviour();
	virtual ~PuckBehaviour();

	Gameplay::Material::Sptr        EnterMaterial;
	Gameplay::Material::Sptr        ExitMaterial;

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	virtual void OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger) override;
	virtual void OnLeavingTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger) override;
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static PuckBehaviour::Sptr FromJson(const nlohmann::json& blob);
	MAKE_TYPENAME(PuckBehaviour);

protected:
	float _speed;
	float _puckY;
	float _paddleY;

	Gameplay::Physics::RigidBody::Sptr _body;
	RenderComponent::Sptr _renderer;
};

