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
	_posX(0.0f),
	_posY(0.0f),
	_speed(1.0f)
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
		_posX = 0.0f;
		LOG_INFO("Entered PuckBehaviour trigger: {}", trigger->GetGameObject()->Name);
	}
	//LOG_INFO("Entered trigger: {}", trigger->GetGameObject()->Name);
}

void PuckBehaviour::OnLeavingTrigger(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
	if (_renderer && ExitMaterial) {
		_renderer->SetMaterial(ExitMaterial);
	}
	/*if (trigger->GetGameObject()->Name == "Paddle P1") {
		_posX = -20;
		LOG_INFO("Entered PuckBehaviour trigger: {}", trigger->GetGameObject()->Name);
	}
	LOG_INFO("Left trigger: {}", trigger->GetGameObject()->Name);*/
}

void PuckBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "posX", &_posX, 0.0f);
	LABEL_LEFT(ImGui::DragFloat, "posY", &_posY, 0.0f);
	LABEL_LEFT(ImGui::DragFloat, "Speed", &_speed, 1.0f);
}

nlohmann::json PuckBehaviour::ToJson() const {
	return {
		{ "posX", _posX },
		{"posY", _posY},
		{"Speed", _speed},
		{ "enter_material", EnterMaterial != nullptr ? EnterMaterial->GetGUID().str() : "null" },
		{ "exit_material", ExitMaterial != nullptr ? ExitMaterial->GetGUID().str() : "null" }
	};
}

PuckBehaviour::Sptr PuckBehaviour::FromJson(const nlohmann::json& blob) {
	PuckBehaviour::Sptr result = std::make_shared<PuckBehaviour>();
	result->EnterMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["enter_material"]));
	result->ExitMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["exit_material"]));
	result->_posX = blob["posX"];
	result->_posY = blob["posY"];
	return result;
}
void PuckBehaviour::Update(float deltaTime) {
	GetGameObject()->SetPostion(glm::vec3(_posX, _posY,1.0f));
	GetGameObject()->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
}