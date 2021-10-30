#include "Gameplay/Components/Player1MovementBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void Player1MovementBehaviour::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	if (_body == nullptr) {
		IsEnabled = false;
	}
}

void Player1MovementBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "posX", &_posX, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "posY", &_posY, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Speed", &_speed, 1.0f);
}

nlohmann::json Player1MovementBehaviour::ToJson() const {
	return {
		{ "posX", _posX },
		{"posY", _posY}
	};
}

Player1MovementBehaviour::Player1MovementBehaviour() :
	IComponent(),
	_posX(-40.0f),
	_posY(0.0f),
	_speed(1.0f)
{ }

Player1MovementBehaviour::~Player1MovementBehaviour() = default;

Player1MovementBehaviour::Sptr Player1MovementBehaviour::FromJson(const nlohmann::json & blob) {
	Player1MovementBehaviour::Sptr result = std::make_shared<Player1MovementBehaviour>();
	result->_posX = blob["posX"];
	result->_posY = blob["posY"];
	return result;
}

void Player1MovementBehaviour::Update(float deltaTime) {
	//Player 1 Controls
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		_posX += _speed;
		if (_posX > -3.0f)
		{
			_posX = -3.0f;			
		}
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		_posX -= _speed;
		if (_posX < -44.0f) {
			_posX = -44.0f;
		}
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_UP) == GLFW_PRESS) {
		_posY -= _speed;
		if (_posY < -44.0f) {
			_posY = -44.0f;
		}
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		_posY += _speed;
		if (_posY > 44.0f) {
			_posY = 44.0f;
		}
	}
	GetGameObject()->SetPostion(glm::vec3(_posX, _posY, 0.0f));
}