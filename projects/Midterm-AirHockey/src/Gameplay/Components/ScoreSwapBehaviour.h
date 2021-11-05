#pragma once
#include "IComponent.h"
#include "Gameplay/Components/RenderComponent.h"
/// <summary>
/// Score Swap Class
/// </summary>
class ScoreSwapBehaviour : public Gameplay::IComponent {
	public:
		typedef std::shared_ptr<ScoreSwapBehaviour> Sptr;

		ScoreSwapBehaviour();
		virtual ~ScoreSwapBehaviour();

		Gameplay::Material::Sptr Text1Material;
		Gameplay::Material::Sptr Text2Material;
		Gameplay::Material::Sptr Text3Material;
		Gameplay::Material::Sptr Text4Material;
		Gameplay::Material::Sptr Text5Material;
		Gameplay::Material::Sptr Text6Material;
		Gameplay::Material::Sptr Text7Material;
		Gameplay::Material::Sptr Text8Material;
		Gameplay::Material::Sptr Text9Material;
		Gameplay::Material::Sptr Text10Material;

		virtual void Awake() override;
		virtual void Update(float deltaTime) override;

		void SwapScore(int WhichMaterial);

		virtual void RenderImGui() override;
		virtual nlohmann::json ToJson() const override;
		static ScoreSwapBehaviour::Sptr FromJson(const nlohmann::json& blob);
		MAKE_TYPENAME(ScoreSwapBehaviour);

	protected:
		RenderComponent::Sptr _renderer;
};