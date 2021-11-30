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
#include "Graphics/TextureCube.h"
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

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"

//#define LOG_GL_NOTIFICATIONS


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
std::string windowTitle = "Assigment 2";

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

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::SaveManifest(newFilename);
	}
	ImGui::SameLine();
	// Load scene from file button
	if (ImGui::Button("Load")) {
		// Since it's a reference to a ptr, this will
		// overwrite the existing scene!
		scene = nullptr;

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::LoadManifest(newFilename);
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

/// <summary>
/// Draws a simple window for displaying materials and their editors
/// </summary>
void DrawMaterialsWindow() {
	if (ImGui::Begin("Materials")) {
		ResourceManager::Each<Material>([](Material::Sptr material) {
			material->RenderImGui();
			});
	}
	ImGui::End();
}

/// <summary>
/// handles creating or loading the scene
/// </summary>
void CreateScene() {
	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene) {
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("scene.json");

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();
	}
	else {
		// This time we'll have 2 different shaders, and share data between both of them using the UBO
		// This shader will handle reflective materials 
		Shader::Sptr reflectiveShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_environment_reflective.glsl" }
		});

		// This shader handles our basic materials without reflections (cause they expensive)
		Shader::Sptr basicShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		});

		// This shader handles our basic materials without reflections (cause they expensive)
		Shader::Sptr specShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/textured_specular.glsl" }
		});

		// This shader handles our cel shading example
		Shader::Sptr toonShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/toon_shading.glsl" }
		});


		///////////////////// NEW SHADERS ////////////////////////////////////////////

		// This shader handles our displacement mapping example
		Shader::Sptr displacementShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});

		// This shader handles our displacement mapping example
		Shader::Sptr tangentSpaceMapping = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});

		// This shader handles our multitexturing example
		Shader::Sptr multiTextureShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});

		Shader::Sptr WaterShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/water.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_water.glsl" }
		});

		//Mesh
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>("plane.obj");

		//Texture
		Texture2D::Sptr	grassTexture = ResourceManager::CreateAsset<Texture2D>("textures/grass.jpg");
		Texture2D::Sptr	HeightMap2 = ResourceManager::CreateAsset<Texture2D>("textures/heightmap2.png");
		Texture2D::Sptr	HeightMap = ResourceManager::CreateAsset<Texture2D>("textures/grassheight.png");
		Texture2D::Sptr	sandTexture = ResourceManager::CreateAsset<Texture2D>("textures/sand.jpg");
		Texture2D::Sptr	snowTexture = ResourceManager::CreateAsset<Texture2D>("textures/snow.jpg");
		Texture2D::Sptr	stoneTexture = ResourceManager::CreateAsset<Texture2D>("textures/stone.jpg");
		Texture2D::Sptr	waterTexture = ResourceManager::CreateAsset<Texture2D>("textures/water.jpg");

		//Create an empty scene
		scene = std::make_shared<Scene>();

		//Materials
		Material::Sptr SandMaterial = ResourceManager::CreateAsset<Material>(displacementShader);
		{

			SandMaterial->Name = "SandMaterial";
			SandMaterial->Set("u_Material.Diffuse", sandTexture);
			SandMaterial->Set("s_Heightmap", HeightMap2);
			SandMaterial->Set("s_NormalMap", sandTexture);
			SandMaterial->Set("u_Material.Shininess", 0.1f);
			SandMaterial->Set("u_Scale", 0.1f);
		}

		Material::Sptr GrassMaterial = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			GrassMaterial->Name = "GrassMaterial";
			GrassMaterial->Set("u_Material.Diffuse", grassTexture);
			GrassMaterial->Set("s_Heightmap", HeightMap2);
			GrassMaterial->Set("s_NormalMap", grassTexture);
			GrassMaterial->Set("u_Material.Shininess", 0.1f);
			GrassMaterial->Set("u_Scale", 0.2f);			
		}
		Material::Sptr StoneMaterial = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			StoneMaterial->Name = "StoneMaterial";
			StoneMaterial->Set("u_Material.Diffuse", stoneTexture);
			StoneMaterial->Set("s_Heightmap", HeightMap2);
			StoneMaterial->Set("s_NormalMap", stoneTexture);
			StoneMaterial->Set("u_Material.Shininess", 0.1f);
			StoneMaterial->Set("u_Scale", 0.3f);
		}
		Material::Sptr SnowMaterial = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			SnowMaterial->Name = "SnowMaterial";
			SnowMaterial->Set("u_Material.Diffuse", snowTexture);
			SnowMaterial->Set("s_Heightmap", HeightMap2);
			SnowMaterial->Set("s_NormalMap", snowTexture);
			SnowMaterial->Set("u_Material.Shininess", 0.1f);
			SnowMaterial->Set("u_Scale", 0.4f);
		}

		Material::Sptr WaterMaterial = ResourceManager::CreateAsset<Material>(WaterShader);
		{
			WaterMaterial->Name = "WaterMaterial";
			WaterMaterial->Set("u_Material.Diffuse", waterTexture);
			WaterMaterial->Set("u_Material.Shininess", 0.5f);
			WaterMaterial->Set("u_Material.Threshold", 0.5f);
		}

		// Create some lights for our scene
		scene->Lights.resize(3);
		scene->Lights[0].Position = glm::vec3(-2.45f, 2.0f, -1.420f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 100.0f;
		scene->Lights[1].Position = glm::vec3(-0.09f, -2.95f, 0.0f);
		scene->Lights[1].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[1].Range = 100.0f;

		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(5.0f));
			camera->LookAt(glm::vec3(0.0f));

			camera->Add<SimpleCameraControl>();

			Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
		}

		// Set up all our sample objects
		GameObject::Sptr SandPlane = scene->CreateGameObject("SandPlane");
		{
			SandPlane->SetScale(glm::vec3(5.0f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = SandPlane->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(SandMaterial);
		}
		GameObject::Sptr WaterPlane = scene->CreateGameObject("WaterPlane");
		{
			WaterPlane->SetScale(glm::vec3(1.8f));
			WaterPlane->SetPostion(glm::vec3(0.0f, 0.0f, 0.03f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = WaterPlane->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(WaterMaterial);
		}
		GameObject::Sptr GrassPlane = scene->CreateGameObject("GrassPlane");
		{
			GrassPlane->SetScale(glm::vec3(5.0f));
			GrassPlane->SetPostion(glm::vec3(0.0f, 0.0f, -0.06f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = GrassPlane->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(GrassMaterial);
		}
		GameObject::Sptr StonePlane = scene->CreateGameObject("StonePlane");
		{
			StonePlane->SetScale(glm::vec3(5.0f));
			StonePlane->SetPostion(glm::vec3(0.0f, 0.0f, -0.31f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = StonePlane->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(StoneMaterial);
		}
		GameObject::Sptr SnowPlane = scene->CreateGameObject("SnowPlane");
		{
			SnowPlane->SetScale(glm::vec3(5.0f));
			SnowPlane->SetPostion(glm::vec3(0.0f, 0.0f, -0.67f));

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = SnowPlane->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(SnowMaterial);
		}
		
		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");
	}
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
			ResourceManager::RegisterType<TextureCube>();
			ResourceManager::RegisterType<Shader>();
			ResourceManager::RegisterType<Material>();
			ResourceManager::RegisterType<MeshResource>();

			// Register all of our component types so we can load them from files
			ComponentManager::RegisterType<Camera>();
			ComponentManager::RegisterType<RenderComponent>();
			ComponentManager::RegisterType<RigidBody>();
			ComponentManager::RegisterType<TriggerVolume>();
			ComponentManager::RegisterType<RotatingBehaviour>();
			ComponentManager::RegisterType<JumpBehaviour>();
			ComponentManager::RegisterType<MaterialSwapBehaviour>();
			ComponentManager::RegisterType<TriggerVolumeEnterBehaviour>();
			ComponentManager::RegisterType<SimpleCameraControl>();

			// GL states, we'll enable depth testing and backface fulling
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

			// Structure for our frame-level uniforms, matches layout from
			// fragments/frame_uniforms.glsl
			// For use with a UBO.
			struct FrameLevelUniforms {
				// The camera's view matrix
				glm::mat4 u_View;
				// The camera's projection matrix
				glm::mat4 u_Projection;
				// The combined viewProject matrix
				glm::mat4 u_ViewProjection;
				// The camera's position in world space
				glm::vec4 u_CameraPos;
				// The time in seconds since the start of the application
				float u_Time;
			};
			// This uniform buffer will hold all our frame level uniforms, to be shared between shaders
			UniformBuffer<FrameLevelUniforms>::Sptr frameUniforms = std::make_shared<UniformBuffer<FrameLevelUniforms>>(BufferUsage::DynamicDraw);
			// The slot that we'll bind our frame level UBO to
			const int FRAME_UBO_BINDING = 0;

			// Structure for our isntance-level uniforms, matches layout from
			// fragments/frame_uniforms.glsl
			// For use with a UBO.
			struct InstanceLevelUniforms {
				// Complete MVP
				glm::mat4 u_ModelViewProjection;
				// Just the model transform, we'll do worldspace lighting
				glm::mat4 u_Model;
				// Normal Matrix for transforming normals
				glm::mat4 u_NormalMatrix;
			};

			// This uniform buffer will hold all our instance level uniforms, to be shared between shaders
			UniformBuffer<InstanceLevelUniforms>::Sptr instanceUniforms = std::make_shared<UniformBuffer<InstanceLevelUniforms>>(BufferUsage::DynamicDraw);

			// The slot that we'll bind our instance level UBO to
			const int INSTANCE_UBO_BINDING = 1;

			////////////////////////////////
			///// SCENE CREATION MOVED /////
			////////////////////////////////
			CreateScene();

			// We'll use this to allow editing the save/load path
			// via ImGui, note the reserve to allocate extra space
			// for input!
			std::string scenePath = "scene.json";
			scenePath.reserve(256);

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

				// Draw our material properties window!
				DrawMaterialsWindow();

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

				// Bind the skybox texture to a reserved texture slot
				// See Material.h and Material.cpp for how we're reserving texture slots
				TextureCube::Sptr environment = scene->GetSkyboxTexture();
				if (environment) environment->Bind(0);

				// Here we'll bind all the UBOs to their corresponding slots
				scene->PreRender();
				frameUniforms->Bind(FRAME_UBO_BINDING);
				instanceUniforms->Bind(INSTANCE_UBO_BINDING);

				// Upload frame level uniforms
				auto& frameData = frameUniforms->GetData();
				frameData.u_Projection = camera->GetProjection();
				frameData.u_View = camera->GetView();
				frameData.u_ViewProjection = camera->GetViewProjection();
				frameData.u_CameraPos = glm::vec4(camera->GetGameObject()->GetPosition(), 1.0f);
				frameData.u_Time = static_cast<float>(thisFrame);
				frameUniforms->Update();

				// Render all our objects
				ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {
					// Early bail if mesh not set
					if (renderable->GetMesh() == nullptr) {
						return;
					}

					// If we don't have a material, try getting the scene's fallback material
					// If none exists, do not draw anything
					if (renderable->GetMaterial() == nullptr) {
						if (scene->DefaultMaterial != nullptr) {
							renderable->SetMaterial(scene->DefaultMaterial);
						}
						else {
							return;
						}
					}

					// If the material has changed, we need to bind the new shader and set up our material and frame data
					// Note: This is a good reason why we should be sorting the render components in ComponentManager
					if (renderable->GetMaterial() != currentMat) {
						currentMat = renderable->GetMaterial();
						shader = currentMat->GetShader();

						shader->Bind();
						currentMat->Apply();
					}

					// Grab the game object so we can do some stuff with it
					GameObject* object = renderable->GetGameObject();

					// Use our uniform buffer for our instance level uniforms
					auto& instanceData = instanceUniforms->GetData();
					instanceData.u_Model = object->GetTransform();
					instanceData.u_ModelViewProjection = viewProj * object->GetTransform();
					instanceData.u_NormalMatrix = glm::mat3(glm::transpose(glm::inverse(object->GetTransform())));
					instanceUniforms->Update();

					// Draw the object
					renderable->GetMesh()->Draw();
					});

				// Use our cubemap to draw our skybox
				scene->DrawSkybox();

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