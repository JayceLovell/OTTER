#include "Gameplay/Components/Player2MovementBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void Player2MovementBehaviour::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	if (_body == nullptr) {
		IsEnabled = false;
	}
}

void Player2MovementBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "posX", &_posX, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "posY", &_posY, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Speed", &_speed, 1.0f);
}

nlohmann::json Player2MovementBehaviour::ToJson() const {
	return {
		{ "posX", _posX },
		{"posY", _posY}
	};
}

Player2MovementBehaviour::Player2MovementBehaviour() :
	IComponent(),
	_posX(40.0f),
	_posY(0.0f),
	_speed(1.0f)
{ }

Player2MovementBehaviour::~Player2MovementBehaviour() = default;

Player2MovementBehaviour::Sptr Player2MovementBehaviour::FromJson(const nlohmann::json & blob) {
	Player2MovementBehaviour::Sptr result = std::make_shared<Player2MovementBehaviour>();
	result->_posX = blob["posX"];
	result->_posY = blob["posY"];
	return result;
}

void Player2MovementBehaviour::Update(float deltaTime) {
	//Player 1 Controls
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A) == GLFW_PRESS) {
		_posX += _speed;
		if (_posX > 44.0f)
		{
			_posX = 44.0f;
		}
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D) == GLFW_PRESS) {
		_posX -= _speed;
		if (_posX < 1.0f) {
			_posX = 1.0f;
		}
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_W) == GLFW_PRESS) {
		_posY -= _speed;
		if (_posY < -44.0f) {
			_posY = -44.0f;
		}
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_S) == GLFW_PRESS) {
		_posY += _speed;
		if (_posY > 44.0f) {
			_posY = 44.0f;
		}
	}
	GetGameObject()->SetPostion(glm::vec3(_posX, _posY, 1.0f));
}