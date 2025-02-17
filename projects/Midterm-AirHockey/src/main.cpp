//Jayce Lovell(100775118) - Jelani Garnes(100801696)
#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <typeindex>
#include <optional>
#include <string>

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

// Graphics
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture2D.h"
#include "Graphics/VertexTypes.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/Player1MovementBehaviour.h"
#include "Gameplay/Components/Player2MovementBehaviour.h"
#include "Gameplay/Components/PuckBehaviour.h"
#include "Gameplay/Components/ScoreSwapBehaviour.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

//#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
	case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
#ifdef LOG_GL_NOTIFICATIONS
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
#endif
	default: break;
	}
}

// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(800, 800);
// The title of our GLFW window
std::string windowTitle = "Midterm - AirHockey - Jayce Lovell(100775118) - Jelani Garnes(100801696) ";

// using namespace should generally be avoided, and if used, make sure it's ONLY in cpp files
using namespace Gameplay;
using namespace Gameplay::Physics;

// The scene that we will be rendering
Scene::Sptr scene = nullptr;

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
	if (windowSize.x * windowSize.y > 0) {
		scene->MainCamera->ResizeWindow(width, height);
	}
}

/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}

/// <summary>
/// Draws a widget for saving or loading our scene
/// </summary>
/// <param name="scene">Reference to scene pointer</param>
/// <param name="path">Reference to path string storage</param>
/// <returns>True if a new scene has been loaded</returns>
bool DrawSaveLoadImGui(Scene::Sptr& scene, std::string& path) {
	// Since we can change the internal capacity of an std::string,
	// we can do cool things like this!
	ImGui::InputText("Path", path.data(), path.capacity());

	// Draw a save button, and save when pressed
	if (ImGui::Button("Save")) {
		scene->Save(path);
	}
	ImGui::SameLine();
	// Load scene from file button
	if (ImGui::Button("Load")) {
		// Since it's a reference to a ptr, this will
		// overwrite the existing scene!
		scene = nullptr;
		scene = Scene::Load(path);

		return true;
	}
	return false;
}

/// <summary>
/// Draws some ImGui controls for the given light
/// </summary>
/// <param name="title">The title for the light's header</param>
/// <param name="light">The light to modify</param>
/// <returns>True if the parameters have changed, false if otherwise</returns>
bool DrawLightImGui(const Scene::Sptr& scene, const char* title, int ix) {
	bool isEdited = false;
	bool result = false;
	Light& light = scene->Lights[ix];
	ImGui::PushID(&light); // We can also use pointers as numbers for unique IDs
	if (ImGui::CollapsingHeader(title)) {
		isEdited |= ImGui::DragFloat3("Pos", &light.Position.x, 0.01f);
		isEdited |= ImGui::ColorEdit3("Col", &light.Color.r);
		isEdited |= ImGui::DragFloat("Range", &light.Range, 0.1f);

		result = ImGui::Button("Delete");
	}
	if (isEdited) {
		scene->SetShaderLight(ix);
	}

	ImGui::PopID();
	return result;
}

int main() {
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	// Initialize our ImGui helper
	ImGuiHelper::Init(window);

	// Initialize our resource manager
	ResourceManager::Init();

	// Register all our resource types so we can load them from manifest files
	ResourceManager::RegisterType<Texture2D>();
	ResourceManager::RegisterType<Material>();
	ResourceManager::RegisterType<MeshResource>();
	ResourceManager::RegisterType<Shader>();

	// Register all of our component types so we can load them from files
	ComponentManager::RegisterType<Camera>();
	ComponentManager::RegisterType<RenderComponent>();
	ComponentManager::RegisterType<RigidBody>();
	ComponentManager::RegisterType<TriggerVolume>();
	ComponentManager::RegisterType<RotatingBehaviour>();
	ComponentManager::RegisterType<JumpBehaviour>();
	ComponentManager::RegisterType<MaterialSwapBehaviour>();
	ComponentManager::RegisterType<Player1MovementBehaviour>();
	ComponentManager::RegisterType<Player2MovementBehaviour>();
	ComponentManager::RegisterType<PuckBehaviour>();
	ComponentManager::RegisterType<ScoreSwapBehaviour>();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene) {
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("scene.json");
	}
	else {
		// Create our OpenGL resources
		Shader::Sptr uboShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" },
			{ ShaderPartType::Fragment, "shaders/frag_blinn_phong_textured.glsl" }
		});

		//Models
		MeshResource::Sptr PuckMesh = ResourceManager::CreateAsset<MeshResource>("Models/Puck.obj");
		MeshResource::Sptr PaddleMesh = ResourceManager::CreateAsset<MeshResource>("Models/paddles.obj");		
		MeshResource::Sptr Wall1Mesh = ResourceManager::CreateAsset<MeshResource>("Models/Wall1.obj");
		MeshResource::Sptr Wall3Mesh = ResourceManager::CreateAsset<MeshResource>("Models/Wall3.obj");


		//Textures
		Texture2D::Sptr PuckTex = ResourceManager::CreateAsset<Texture2D>("textures/puckTexture.jpg");		
		Texture2D::Sptr PaddleP1Tex = ResourceManager::CreateAsset<Texture2D>("textures/red.jpg");
		Texture2D::Sptr PaddleP2Tex = ResourceManager::CreateAsset<Texture2D>("textures/blue.jpg");
		Texture2D::Sptr PlaneTexture = ResourceManager::CreateAsset<Texture2D>("textures/Plane.jpg");
		Texture2D::Sptr WallTexture = ResourceManager::CreateAsset<Texture2D>("textures/walltexture.jpg");
		Texture2D::Sptr Text0 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/0.jpg");
		Texture2D::Sptr Text1 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/1.jpg");
		Texture2D::Sptr Text2 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/2.jpg");
		Texture2D::Sptr Text3 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/3.jpg");
		Texture2D::Sptr Text4 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/4.jpg");
		Texture2D::Sptr Text5 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/5.jpg");
		Texture2D::Sptr Text6 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/6.jpg");
		Texture2D::Sptr Text7 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/7.jpg");
		Texture2D::Sptr Text8 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/8.jpg");
		Texture2D::Sptr Text9 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/9.jpg");
		Texture2D::Sptr Text10 = ResourceManager::CreateAsset<Texture2D>("textures/numbers/10.jpg");

		// Create an empty scene
		scene = std::make_shared<Scene>();
		

		// I hate this
		scene->BaseShader = uboShader;

		// Create our materials
		Material::Sptr planeMaterial = ResourceManager::CreateAsset<Material>();
		{
			planeMaterial->Name = "Plane";
			planeMaterial->MatShader = scene->BaseShader;
			planeMaterial->Texture = PlaneTexture;
			planeMaterial->Shininess = 2.0f;
		}

		Material::Sptr Text0Material = ResourceManager::CreateAsset<Material>();
		{
			Text0Material->Name = "Text0Material";
			Text0Material->MatShader = scene->BaseShader;
			Text0Material->Texture = Text0;
			Text0Material->Shininess = 2.0f;
		}
		Material::Sptr Text1Material = ResourceManager::CreateAsset<Material>();
		{
			Text1Material->Name = "Text1Material";
			Text1Material->MatShader = scene->BaseShader;
			Text1Material->Texture = Text1;
			Text1Material->Shininess = 2.0f;
		}
		Material::Sptr Text2Material = ResourceManager::CreateAsset<Material>();
		{
			Text2Material->Name = "Text2Material";
			Text2Material->MatShader = scene->BaseShader;
			Text2Material->Texture = Text2;
			Text2Material->Shininess = 2.0f;
		}
		Material::Sptr Text3Material = ResourceManager::CreateAsset<Material>();
		{
			Text3Material->Name = "Text3Material";
			Text3Material->MatShader = scene->BaseShader;
			Text3Material->Texture = Text3;
			Text3Material->Shininess = 2.0f;
		}
		Material::Sptr Text4Material = ResourceManager::CreateAsset<Material>();
		{
			Text4Material->Name = "Text4Material";
			Text4Material->MatShader = scene->BaseShader;
			Text4Material->Texture = Text4;
			Text4Material->Shininess = 2.0f;
		}
		Material::Sptr Text5Material = ResourceManager::CreateAsset<Material>();
		{
			Text5Material->Name = "Text5Material";
			Text5Material->MatShader = scene->BaseShader;
			Text5Material->Texture = Text5;
			Text5Material->Shininess = 2.0f;
		}
		Material::Sptr Text6Material = ResourceManager::CreateAsset<Material>();
		{
			Text6Material->Name = "Text6Material";
			Text6Material->MatShader = scene->BaseShader;
			Text6Material->Texture = Text6;
			Text6Material->Shininess = 2.0f;
		}
		Material::Sptr Text7Material = ResourceManager::CreateAsset<Material>();
		{
			Text7Material->Name = "Text7Material";
			Text7Material->MatShader = scene->BaseShader;
			Text7Material->Texture = Text7;
			Text7Material->Shininess = 2.0f;
		}
		Material::Sptr Text8Material = ResourceManager::CreateAsset<Material>();
		{
			Text8Material->Name = "Text8Material";
			Text8Material->MatShader = scene->BaseShader;
			Text8Material->Texture = Text8;
			Text8Material->Shininess = 2.0f;
		}
		Material::Sptr Text9Material = ResourceManager::CreateAsset<Material>();
		{
			Text9Material->Name = "Text9Material";
			Text9Material->MatShader = scene->BaseShader;
			Text9Material->Texture = Text9;
			Text9Material->Shininess = 2.0f;
		}
		Material::Sptr Text10Material = ResourceManager::CreateAsset<Material>();
		{
			Text10Material->Name = "Text10Material";
			Text10Material->MatShader = scene->BaseShader;
			Text10Material->Texture = Text10;
			Text10Material->Shininess = 2.0f;
		}


		Material::Sptr PuckMaterial = ResourceManager::CreateAsset<Material>();
		{
			PuckMaterial->Name = "PuckTex";
			PuckMaterial->MatShader = scene->BaseShader;
			PuckMaterial->Texture = PuckTex;
			PuckMaterial->Shininess = 2.0f;
		}

		Material::Sptr PaddleP1Material = ResourceManager::CreateAsset<Material>();
		{
			PaddleP1Material->Name = "PaddleP1Tex";
			PaddleP1Material->MatShader = scene->BaseShader;
			PaddleP1Material->Texture = PaddleP1Tex;
			PaddleP1Material->Shininess = 256.0f;
		}

		Material::Sptr PaddleP2Material = ResourceManager::CreateAsset<Material>();
		{
			PaddleP2Material->Name = "PaddleP2Tex";
			PaddleP2Material->MatShader = scene->BaseShader;
			PaddleP2Material->Texture = PaddleP2Tex;
			PaddleP2Material->Shininess = 256.0f;
		}

		Material::Sptr WallMaterial = ResourceManager::CreateAsset<Material>();
		{
			WallMaterial->Name = "WallTexture";
			WallMaterial->MatShader = scene->BaseShader;
			WallMaterial->Texture = WallTexture;
			WallMaterial->Shininess = 2.0f;
		}



		// Create some lights for our scene
		scene->Lights.resize(7);
		scene->Lights[0].Position = glm::vec3(0.0f, 1.0f, 50.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 1000.0f;

		scene->Lights[1].Position = glm::vec3(48.0f, 0.0f, 1.0f);
		scene->Lights[1].Color = glm::vec3(0.0f, 0.0f, 0.255f);
		scene->Lights[1].Range = 35.0f;

		scene->Lights[2].Position = glm::vec3(-48.0f, 0.0f, 1.0f);
		scene->Lights[2].Color = glm::vec3(0.373f, 0.0f, 0.0f);
		scene->Lights[2].Range = 35.0f;

		scene->Lights[3].Position = glm::vec3(36.0f, 46.0f, 14.0f);
		scene->Lights[3].Color = glm::vec3(0.255f, 0.255f, 0.255f);
		scene->Lights[3].Range = 73.0f;

		scene->Lights[4].Position = glm::vec3(-36.0f, 46.0f, 14.0f);
		scene->Lights[4].Color = glm::vec3(0.255f, 0.255f, 0.255f);
		scene->Lights[4].Range = 73.0f;
		//Lights for score
		scene->Lights[5].Position = glm::vec3(-36.0f, -46.0f, 14.0f);
		scene->Lights[5].Color = glm::vec3(0.255f, 0.255f, 0.255f);
		scene->Lights[5].Range = 73.0f;

		scene->Lights[6].Position = glm::vec3(36.0f, -46.0f, 14.0f);
		scene->Lights[6].Color = glm::vec3(0.255f, 0.255f, 0.255f);
		scene->Lights[6].Range = 73.0f;

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();
		
		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(0, 1, 60));
			camera->LookAt(glm::vec3(0.0f));

			Camera::Sptr cam = camera->Add<Camera>();

			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
		}

		// Set up all our sample objects
			GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			// Scale up the plane
			plane->SetScale(glm::vec3(100.0f));

			//Rotate it horizontally
			plane->SetRotation(glm::vec3(0.0f, 0.0f, 90.0f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(planeMaterial);

			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(PlaneCollider::Create());

			// This object is a renderable only, it doesn't have any behaviours or
			// physics bodies attached!
		}

		//Top-wall
		GameObject::Sptr Wall1 = scene->CreateGameObject("Wall1"); 
		{			
			Wall1->SetPostion(glm::vec3(0.0f,-47.980f,0.760f));			

			TriggerVolume::Sptr volume = Wall1->Add<TriggerVolume>();
			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Wall1->Add<RenderComponent>();
			renderer->SetMesh(Wall1Mesh);
			renderer->SetMaterial(WallMaterial);

			// Attach a wall collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = Wall1->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(ConvexMeshCollider::Create());
			volume->AddCollider(ConvexMeshCollider::Create());
		}

		//Bottom-wall
		GameObject::Sptr Wall2 = scene->CreateGameObject("Wall2");
		{
			Wall2->SetPostion(glm::vec3(0.0f, 47.980f, 0.760f));

			TriggerVolume::Sptr volume = Wall2->Add<TriggerVolume>();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Wall2->Add<RenderComponent>();
			renderer->SetMesh(Wall1Mesh);
			renderer->SetMaterial(WallMaterial);

			// Attach a wall collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = Wall2->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(ConvexMeshCollider::Create());
			volume->AddCollider(ConvexMeshCollider::Create());
		}

		//Top-Left wall
		GameObject::Sptr Wall3 = scene->CreateGameObject("Wall3");
		{

			Wall3->SetPostion(glm::vec3(49.110f, -27.710f, 0.760f));

			TriggerVolume::Sptr volume = Wall3->Add<TriggerVolume>();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Wall3->Add<RenderComponent>();
			renderer->SetMesh(Wall3Mesh);
			renderer->SetMaterial(WallMaterial);

			// Attach a wall collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = Wall3->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(ConvexMeshCollider::Create());
			volume->AddCollider(ConvexMeshCollider::Create());
		}

		//Bottom-left wall
		GameObject::Sptr Wall4 = scene->CreateGameObject("Wall4");
		{

			Wall4->SetPostion(glm::vec3(49.110f, 27.710f, 0.760f));

			TriggerVolume::Sptr volume = Wall4->Add<TriggerVolume>();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Wall4->Add<RenderComponent>();
			renderer->SetMesh(Wall3Mesh);
			renderer->SetMaterial(WallMaterial);

			// Attach a wall collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = Wall4->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(ConvexMeshCollider::Create());
			volume->AddCollider(ConvexMeshCollider::Create());
		}

		//Top-right wall
		GameObject::Sptr Wall5 = scene->CreateGameObject("Wall5");
		{

			Wall5->SetPostion(glm::vec3(-49.110f, -27.710f, 0.760f));

			TriggerVolume::Sptr volume = Wall5->Add<TriggerVolume>();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Wall5->Add<RenderComponent>();
			renderer->SetMesh(Wall3Mesh);
			renderer->SetMaterial(WallMaterial);

			// Attach a wall collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = Wall5->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(ConvexMeshCollider::Create());
			volume->AddCollider(ConvexMeshCollider::Create());
		}

		//Bottom-right wall
		GameObject::Sptr Wall6 = scene->CreateGameObject("Wall6");
		{

			Wall6->SetPostion(glm::vec3(-49.110f, 27.710f, 0.760f));
			TriggerVolume::Sptr volume = Wall6->Add<TriggerVolume>();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Wall6->Add<RenderComponent>();
			renderer->SetMesh(Wall3Mesh);
			renderer->SetMaterial(WallMaterial);

			// Attach a wall collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = Wall6 ->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(ConvexMeshCollider::Create());
			volume->AddCollider(ConvexMeshCollider::Create());
		}

		GameObject::Sptr Puck = scene->CreateGameObject("Puck");
		{
			// Set position in the scene
			Puck->SetPostion(glm::vec3(0.0f, 0.0f, 10.0f));
			//Puck->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			//Puck->SetScale(glm::vec3(2.0f, 2.0f, 1));
			

			// Add some behaviour that relies on the physics body
			//monkey1->Add<JumpBehaviour>();
			Puck->Add<PuckBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = Puck->Add<RenderComponent>();
			renderer->SetMesh(PuckMesh);
			renderer->SetMaterial(PuckMaterial);

			// Add a dynamic rigid body to this monkey
			RigidBody::Sptr physics = Puck->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(ConvexMeshCollider::Create());
			

			 /*We'll add a behaviour that will interact with our trigger volumes*/
			MaterialSwapBehaviour::Sptr triggerInteraction = Puck->Add<MaterialSwapBehaviour>();
			triggerInteraction->EnterMaterial = PuckMaterial;
			triggerInteraction->ExitMaterial = PuckMaterial;
			

			// This is an example of attaching a component and setting some parameters
			RotatingBehaviour::Sptr behaviour = Puck->Add<RotatingBehaviour>();
			behaviour->RotationSpeed = glm::vec3(0.0f, 0.0f, -90.0f);
		}

		GameObject::Sptr PaddleP1 = scene->CreateGameObject("Paddle P1");
		{
			// Set position in the scene
			PaddleP1->SetPostion(glm::vec3(-40.0f, 0.0f, 0.0f));
			PaddleP1->SetRotation(glm::vec3(0.0, 90.0, 0.0));

			// Add some behaviour that relies on the physics body
			PaddleP1->Add<Player1MovementBehaviour>();
			TriggerVolume::Sptr volume = PaddleP1->Add<TriggerVolume>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = PaddleP1->Add<RenderComponent>();
			renderer->SetMesh(PaddleMesh);
			renderer->SetMaterial(PaddleP1Material);						

			// Add a dynamic rigid body to this monkey
			RigidBody::Sptr physics = PaddleP1->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(ConvexMeshCollider::Create());			
			volume->AddCollider(ConvexMeshCollider::Create());

			// We'll add a behaviour that will interact with our trigger volumes
			/*MaterialSwapBehaviour::Sptr triggerInteraction = PaddleP1->Add<MaterialSwapBehaviour>();
			triggerInteraction->EnterMaterial = PaddleP1Material;
			triggerInteraction->ExitMaterial = PaddleP1Material;*/
		}

		GameObject::Sptr PaddleP2 = scene->CreateGameObject("Paddle P2");
		{
			// Set position in the scene
			PaddleP2->SetPostion(glm::vec3(40.0f, 0.0f, 0.0f));
			PaddleP2->SetRotation(glm::vec3(0.0, 90.0, 0.0));

			// Add some behaviour that relies on the physics body
			PaddleP2->Add<Player2MovementBehaviour>();
			TriggerVolume::Sptr volume = PaddleP2->Add<TriggerVolume>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = PaddleP2->Add<RenderComponent>();
			renderer->SetMesh(PaddleMesh);
			renderer->SetMaterial(PaddleP2Material);

			// Add a dynamic rigid body to this monkey
			RigidBody::Sptr physics = PaddleP2->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(ConvexMeshCollider::Create());

			volume->AddCollider(ConvexMeshCollider::Create());

			// We'll add a behaviour that will interact with our trigger volumes
			/*MaterialSwapBehaviour::Sptr triggerInteraction = PaddleP2->Add<MaterialSwapBehaviour>();
			triggerInteraction->EnterMaterial = PaddleP2Material;
			triggerInteraction->ExitMaterial = PaddleP2Material;*/
		}
		GameObject::Sptr GoalRightSide = scene->CreateGameObject("GoalRightSide");
		{
			TriggerVolume::Sptr volume = GoalRightSide->Add<TriggerVolume>();
			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(3.0f, 6.0f, 1.0f));
			collider->SetPosition(glm::vec3(-50.0f, 0.0f, 0.f));
			volume->AddCollider(collider);
		}
		GameObject::Sptr GoalLeftSide = scene->CreateGameObject("GoalLeftSide");
		{
			TriggerVolume::Sptr volume = GoalLeftSide->Add<TriggerVolume>();
			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(3.0f, 6.0f, 1.0f));
			collider->SetPosition(glm::vec3(50.0f, 0.0f, 0.f));
			volume->AddCollider(collider);
		}

		GameObject::Sptr Score1 = scene->CreateGameObject("Score1");
		{
			// Scale up the plane
			Score1->SetScale(glm::vec3(10.0f));

			Score1->SetPostion(glm::vec3(-36.0f,-45.0f,10.0f));

			//Rotate it horizontally
			Score1->SetRotation(glm::vec3(0.0f, 0.0f, 180.0f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Score1->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(Text0Material);


			ScoreSwapBehaviour::Sptr ScoreSwap = Score1->Add<ScoreSwapBehaviour>();
			ScoreSwap->Text1Material = Text1Material;
			ScoreSwap->Text2Material = Text2Material;
			ScoreSwap->Text3Material = Text3Material;
			ScoreSwap->Text4Material = Text4Material;
			ScoreSwap->Text5Material = Text5Material;
			ScoreSwap->Text6Material = Text6Material;
			ScoreSwap->Text7Material = Text7Material;
			ScoreSwap->Text8Material = Text8Material;
			ScoreSwap->Text9Material = Text9Material;
			ScoreSwap->Text10Material = Text10Material;
		}
		GameObject::Sptr Score2 = scene->CreateGameObject("Score2");
		{
			// Scale up the plane
			Score2->SetScale(glm::vec3(10.0f));

			Score2->SetPostion(glm::vec3(36.0f, -45.0f, 10.0f));

			//Rotate it horizontally
			Score2->SetRotation(glm::vec3(0.0f, 0.0f, 180.0f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = Score2->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(Text0Material);

			ScoreSwapBehaviour::Sptr ScoreSwap = Score2->Add<ScoreSwapBehaviour>();			
			ScoreSwap->Text1Material = Text1Material;
			ScoreSwap->Text2Material = Text2Material;
			ScoreSwap->Text3Material = Text3Material;
			ScoreSwap->Text4Material = Text4Material;
			ScoreSwap->Text5Material = Text5Material;
			ScoreSwap->Text6Material = Text6Material;
			ScoreSwap->Text7Material = Text7Material;
			ScoreSwap->Text8Material = Text8Material;
			ScoreSwap->Text9Material = Text9Material;
			ScoreSwap->Text10Material = Text10Material;
		}

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");
	}

	// Call scene awake to start up all of our components
	scene->Window = window;
	scene->Awake();

	// We'll use this to allow editing the save/load path
	// via ImGui, note the reserve to allocate extra space
	// for input!
	std::string scenePath = "scene.json";
	scenePath.reserve(256);

	bool isRotating = true;
	float rotateSpeed = 90.0f;

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	BulletDebugMode physicsDebugMode = BulletDebugMode::None;
	float playbackSpeed = 1.0f;

	nlohmann::json editorSceneState;

	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGuiHelper::StartFrame();

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		scene->IsPlaying = true;

		// Showcasing how to use the imGui library!
		bool isDebugWindowOpen = ImGui::Begin("Debugging");
		if (isDebugWindowOpen) {
			// Draws a button to control whether or not the game is currently playing
			static char buttonLabel[64];
			sprintf_s(buttonLabel, "%s###playmode", scene->IsPlaying ? "Exit Play Mode" : "Enter Play Mode");
			if (ImGui::Button(buttonLabel)) {
				// Save scene so it can be restored when exiting play mode
				if (!scene->IsPlaying) {
					editorSceneState = scene->ToJson();
				}

				// Toggle state
				scene->IsPlaying = !scene->IsPlaying;

				// If we've gone from playing to not playing, restore the state from before we started playing
				if (!scene->IsPlaying) {
					scene = nullptr;
					// We reload to scene from our cached state
					scene = Scene::FromJson(editorSceneState);
					// Don't forget to reset the scene's window and wake all the objects!
					scene->Window = window;
					scene->Awake();
				}
			}

			// Make a new area for the scene saving/loading
			ImGui::Separator();
			if (DrawSaveLoadImGui(scene, scenePath)) {
				/*scene->Save(scenePath);
				std::string newFilename = std::filesystem::path(scenePath).stem().string() + "-manifest.json";
				ResourceManager::SaveManifest(newFilename);

				std::string newFilename = std::filesystem::path(scenePath).stem().string() + "-manifest.json";
				ResourceManager::LoadManifest(newFilename);*/
				scene = Scene::Load(scenePath);

				// C++ strings keep internal lengths which can get annoying
				// when we edit it's underlying datastore, so recalcualte size
				scenePath.resize(strlen(scenePath.c_str()));

				// We have loaded a new scene, call awake to set
				// up all our components
				scene->Window = window;
				scene->Awake();
			}
			ImGui::Separator();
			// Draw a dropdown to select our physics debug draw mode
			if (BulletDebugDraw::DrawModeGui("Physics Debug Mode:", physicsDebugMode)) {
				scene->SetPhysicsDebugDrawMode(physicsDebugMode);
			}
			LABEL_LEFT(ImGui::SliderFloat, "Playback Speed:    ", &playbackSpeed, 0.0f, 10.0f);
			ImGui::Separator();
		}

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update our application level uniforms every frame

		// Draw some ImGui stuff for the lights
		if (isDebugWindowOpen) {
				for (int ix = 0; ix < scene->Lights.size(); ix++) {
					char buff[256];
					sprintf_s(buff, "Light %d##%d", ix, ix);
					// DrawLightImGui will return true if the light was deleted
					if (DrawLightImGui(scene, buff, ix)) {
						// Remove light from scene, restore all lighting data
						scene->Lights.erase(scene->Lights.begin() + ix);
						scene->SetupShaderAndLights();
						// Move back one element so we don't skip anything!
						ix--;
					}
				}
				// As long as we don't have max lights, draw a button
				// to add another one
				if (scene->Lights.size() < scene->MAX_LIGHTS) {
					if (ImGui::Button("Add Light")) {
						scene->Lights.push_back(Light());
						scene->SetupShaderAndLights();
					}
				}
				// Split lights from the objects in ImGui
				ImGui::Separator();
			}

		dt *= playbackSpeed;

		// Perform updates for all components
		scene->Update(dt);

		// Grab shorthands to the camera and shader from the scene
		Camera::Sptr camera = scene->MainCamera;

		// Cache the camera's viewprojection
		glm::mat4 viewProj = camera->GetViewProjection();
		DebugDrawer::Get().SetViewProjection(viewProj);

		// Update our worlds physics!
		scene->DoPhysics(dt);

		// Draw object GUIs
		if (isDebugWindowOpen) {
			scene->DrawAllGameObjectGUIs();
		}

		// The current material that is bound for rendering
		Material::Sptr currentMat = nullptr;
		Shader::Sptr shader = nullptr;

		// Render all our objects
		ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {

			// If the material has changed, we need to bind the new shader and set up our material and frame data
			// Note: This is a good reason why we should be sorting the render components in ComponentManager
			if (renderable->GetMaterial() != currentMat) {
				currentMat = renderable->GetMaterial();
				shader = currentMat->MatShader;

				shader->Bind();
				shader->SetUniform("u_CamPos", scene->MainCamera->GetGameObject()->GetPosition());
				currentMat->Apply();
			}

			// Grab the game object so we can do some stuff with it
			GameObject* object = renderable->GetGameObject();

			// Set vertex shader parameters
			shader->SetUniformMatrix("u_ModelViewProjection", viewProj * object->GetTransform());
			shader->SetUniformMatrix("u_Model", object->GetTransform());
			shader->SetUniformMatrix("u_NormalMatrix", glm::mat3(glm::transpose(glm::inverse(object->GetTransform()))));

			// Draw the object
			renderable->GetMesh()->Draw();
			});

		// End our ImGui window
		ImGui::End();

		VertexArrayObject::Unbind();

		lastFrame = thisFrame;
		ImGuiHelper::EndFrame();
		glfwSwapBuffers(window);
	}
	// Clean up the ImGui library
	ImGuiHelper::Cleanup();

	// Clean up the resource manager
	ResourceManager::Cleanup();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}