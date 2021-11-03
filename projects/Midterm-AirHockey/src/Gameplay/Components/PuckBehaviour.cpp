#include "Gameplay/Components/PuckBehaviour.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

PuckBehaviour::PuckBehaviour() :
	IComponent(),
	_renderer(nullptr),
	EnterMaterial(nullptr),
	ExitMaterial(nullptr),
	_speed(10.0f)
{}
PuckBehaviour::~PuckBehaviour() = default;

void PuckBehaviour::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	_renderer = GetComponent<RenderComponent>();
	if (_body == nullptr) {
		IsEnabled = false;
	}
}

void PuckBehaviour::OnEnteredTrigger(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
	if (_renderer && EnterMaterial) {
		_renderer->SetMaterial(EnterMaterial);
	}
	
	if (trigger->GetGameObject()->Name == "Paddle P1") {
		//First get paddle's Y
		_paddleY = trigger->GetGameObject()->GetPosition().y;

		//Depend on Paddle's Y is where we add the impulse
		if (_paddleY < _puckY) {
			LOG_INFO("Paddle P1 hit from below");
			_body->ApplyImpulse(glm::vec3(_speed, _speed, 0.0f));
		}
		if (_paddleY > _puckY) {
			LOG_INFO("Paddle P1 hit from above");
			_body->ApplyImpulse(glm::vec3(_speed, -_speed, 0.0f));
		}
	}
	if (trigger->GetGameObject()->Name == "Paddle P2") {
		//First get paddle's Y
		_paddleY = trigger->GetGameObject()->GetPosition().y;

		//Depend on Paddle's Y is where we add the impulse
		if (_paddleY < _puckY) {
			LOG_INFO("Paddle P2 hit from below");
			_body->ApplyImpulse(glm::vec3(-_speed, _speed, 0.0f));
		}
		if (_paddleY > _puckY) {
			LOG_INFO("Paddle P2 hit from above");
			_body->ApplyImpulse(glm::vec3(-_speed, -_speed, 0.0f));
		}
	}
	//LOG_INFO("Entered trigger: {}", trigger->GetGameObject()->Name);
}

void PuckBehaviour::OnLeavingTrigger(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
	if (_renderer && ExitMaterial) {
		_renderer->SetMaterial(ExitMaterial);
	}
	
	//LOG_INFO("Left trigger: {}", trigger->GetGameObject()->Name);
}

void PuckBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Speed", &_speed, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Puck-Y", &_puckY, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Paddle-Y", &_paddleY, 1.0f);
}

nlohmann::json PuckBehaviour::ToJson() const {
	return {
		{"Speed", _speed},
		{"Puck-Y",_puckY},
		{"Paddle-Y",_paddleY},
		{ "enter_material", EnterMaterial != nullptr ? EnterMaterial->GetGUID().str() : "null" },
		{ "exit_material", ExitMaterial != nullptr ? ExitMaterial->GetGUID().str() : "null" }
	};
}

PuckBehaviour::Sptr PuckBehaviour::FromJson(const nlohmann::json& blob) {
	PuckBehaviour::Sptr result = std::make_shared<PuckBehaviour>();
	result->EnterMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["enter_material"]));
	result->ExitMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["exit_material"]));
	return result;
}
void PuckBehaviour::Update(float deltaTime) {
	_puckY = GetGameObject()->GetPosition().y;
	//GetGameObject()->SetPostion(glm::vec3(_posX, _posY,1.0f));
	//GetGameObject()->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
}