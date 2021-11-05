#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"

MaterialSwapBehaviour::MaterialSwapBehaviour() :
	IComponent(),
	_renderer(nullptr),
	EnterMaterial(nullptr),
	ExitMaterial(nullptr)
{ }
MaterialSwapBehaviour::~MaterialSwapBehaviour() = default;

void MaterialSwapBehaviour::OnEnteredTrigger(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
	if (_renderer && EnterMaterial) {
		_renderer->SetMaterial(EnterMaterial);
	}
	//LOG_INFO("Entered trigger: {}", trigger->GetGameObject()->Name);
}

void MaterialSwapBehaviour::OnLeavingTrigger(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
	if (_renderer && ExitMaterial) {
		_renderer->SetMaterial(ExitMaterial);
	}
	//LOG_INFO("Left trigger: {}", trigger->GetGameObject()->Name);
}

void MaterialSwapBehaviour::Awake() {
	_renderer = GetComponent<RenderComponent>();
}

void MaterialSwapBehaviour::RenderImGui() { }

nlohmann::json MaterialSwapBehaviour::ToJson() const {
	return {
		{ "enter_material", EnterMaterial != nullptr ? EnterMaterial->GetGUID().str() : "null" },
		{ "exit_material", ExitMaterial != nullptr ? ExitMaterial->GetGUID().str() : "null" },
		{ "Text1_material", Text1Material != nullptr ? Text1Material->GetGUID().str() : "null" },
		{ "Text2_material", Text2Material != nullptr ? Text2Material->GetGUID().str() : "null" },
		{ "Text3_material", Text3Material != nullptr ? Text3Material->GetGUID().str() : "null" },
		{ "Text4_material", Text4Material != nullptr ? Text4Material->GetGUID().str() : "null" },
		{ "Text5_material", Text5Material != nullptr ? Text5Material->GetGUID().str() : "null" },
		{ "Text6_material", Text6Material != nullptr ? Text6Material->GetGUID().str() : "null" },
		{ "Text7_material", Text7Material != nullptr ? Text7Material->GetGUID().str() : "null" },
		{ "Text8_material", Text8Material != nullptr ? Text8Material->GetGUID().str() : "null" },
		{ "Text9_material", Text9Material != nullptr ? Text9Material->GetGUID().str() : "null" },
		{ "Text10_material", Text10Material != nullptr ? Text10Material->GetGUID().str() : "null" }
	};
}

/// <summary>
/// /Switch score material DON'T KNOW WHY IT WON'T CALL
/// </summary>
/// <param name="WhichMaterial"></param>
void MaterialSwapBehaviour::SwapScore(int WhichMaterial)
{
	LOG_INFO("Swaping Score with value {}", WhichMaterial);
	switch (WhichMaterial) {
	case 1:
		_renderer->SetMaterial(Text1Material);
	case 2:
		_renderer->SetMaterial(Text2Material);
	case 3:
		_renderer->SetMaterial(Text3Material);
	case 4:
		_renderer->SetMaterial(Text4Material);
	case 5:
		_renderer->SetMaterial(Text5Material);
	case 6:
		_renderer->SetMaterial(Text6Material);
	case 7:
		_renderer->SetMaterial(Text7Material);
	case 8:
		_renderer->SetMaterial(Text8Material);
	case 9:
		_renderer->SetMaterial(Text9Material);
	case 10:
		_renderer->SetMaterial(Text10Material);
	}
}

MaterialSwapBehaviour::Sptr MaterialSwapBehaviour::FromJson(const nlohmann::json& blob) {
	MaterialSwapBehaviour::Sptr result = std::make_shared<MaterialSwapBehaviour>();
	result->EnterMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["enter_material"]));
	result->ExitMaterial  = ResourceManager::Get<Gameplay::Material>(Guid(blob["exit_material"]));
	result->Text1Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test1_material"]));
	result->Text2Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test2_material"]));
	result->Text3Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test3_material"]));
	result->Text4Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test4_material"]));
	result->Text5Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test5_material"]));
	result->Text6Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test6_material"]));
	result->Text7Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test7_material"]));
	result->Text8Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test8_material"]));
	result->Text9Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test9_material"]));
	result->Text10Material = ResourceManager::Get<Gameplay::Material>(Guid(blob["test10_material"]));

	return result;
}
