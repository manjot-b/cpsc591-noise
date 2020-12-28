#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <iostream>
#include <filesystem>
#include <cstdlib>

#include "Renderer.h"

Renderer::Renderer(int seed) :
	logs(3), demoModels(4),
	showCursor(false), rotate(0), scale(1), camera(glm::vec3(0,5,12)),
	firstMouse(true), lastX(width / 2.0f), lastY(height / 2.0f),
	shiftPressed(false), deltaTime(0.0f), lastFrame(0.0f)
{
	initWindow();
	initImGui();
	shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
	shader->link();
	loadModels();	
	setupModels();
	
	perspective = glm::perspective(glm::radians(45.0f), float(width)/height, 0.1f, 100.0f);
	shader->use();
	shader->setUniformMatrix4fv("perspective", perspective);
	shader->setUniformMatrix4fv("view", camera.getViewMatrix());

	glm::vec3 lightPos = glm::vec3(-5.0, 25.0, 20.0);
	shader->setUniform3fv("lightPos", lightPos);

	int perm[256];
	for (unsigned int i = 0; i < 256; i++)
		perm[i] = i;
	shuffle(perm, seed);
	shader->setUniform1iv("perm", 256, perm);
	shader->setUniform1iv("perm[256]", 256, perm);

	glUseProgram(0);	// unbind shader
}

Renderer::~Renderer() {}

void Renderer::shuffle(int perm[256], int seed)
{
	std::srand(seed);
	for(unsigned int i = 0; i < 256; i++)
	{
		int rand = std::rand() % 256;
		int tmp = perm[i];
		perm[i] = perm[rand];
		perm[rand] = tmp;
	}
}

void Renderer::initWindow()
{
	// Setup glfw context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "OpenGL Example", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	glViewport(0, 0, width, height);

	// Let GLFW store pointer to this instance of Renderer.
	glfwSetWindowUserPointer(window, static_cast<void*>(this));

	glfwSetFramebufferSizeCallback(window,
			[](GLFWwindow* window, int newWidth, int newHeight) {

		Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

		glViewport(0, 0, newWidth, newHeight);
		renderer->perspective = glm::perspective(glm::radians(45.0f), float(newWidth)/newHeight, 0.1f, 100.0f);
	});
 
	glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void Renderer::initImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Renderer::loadModels()
{
	namespace fs = std::filesystem;
	const std::string dir = "models/";

	const std::string logPath = dir + "log.obj";
	const std::string terrainPath = dir + "terrain-3.obj";
	const std::string waterPath = dir + "water.obj";

	const std::string cubePath = dir + "cube.obj";
	const std::string spherePath = dir + "sphere.obj";
	const std::string teapotPath = dir + "teapot.obj";
	const std::string bunnyPath = dir + "bunny.obj";

	for (auto& log : logs)
	{
		loadModel(logPath, log);
	}

	loadModel(waterPath, water);
	loadModel(terrainPath, terrain);

	loadModel(cubePath, demoModels[0]);
	loadModel(spherePath, demoModels[1]);
	loadModel(teapotPath, demoModels[2]);
	loadModel(bunnyPath, demoModels[3]);


	models.push_back(water);
	models.push_back(terrain);
	for (auto& log : logs)
	{
		models.push_back(log);
	}

	for (auto& model : demoModels)
	{
		models.push_back(model);
	}
}

void Renderer::loadModel(const std::string path, std::shared_ptr<Model>& model)
{
	std::cout << "Loading " << path << "..." << std::flush;
	model = std::make_shared<Model>(path);
	std::cout << "Done!\n"; 
}

void Renderer::setupModels()
{
	// Water
	water->fragmentSettings.noiseEffect = Model::NoiseType::WATER;
	water->fragmentSettings.phaseSpeed = 1.4;
	water->fragmentSettings.minFrequency = 50;
	water->fragmentSettings.maxFrequency = 200;
	water->fragmentSettings.waveCenters = 20;
	water->scale(20);
	water->translate(glm::vec3(0,-2.77,0));

	// Terrain
	terrain->fragmentSettings.noiseEffect = Model::NoiseType::GRASS;
	terrain->fragmentSettings.persistence = 7/16.0f;
	terrain->fragmentSettings.octaveCount = 4;
	terrain->fragmentSettings.octaveStart = 1;
	terrain->scale(10);

	// Logs
	for(auto& log : logs)
	{
		log->fragmentSettings.noiseEffect = Model::NoiseType::WOOD;
		log->fragmentSettings.persistence = 2/16.0f;
		log->fragmentSettings.ringFrequency = 80;
		log->fragmentSettings.octaveCount = 3;
		log->fragmentSettings.octaveStart = 0;
	}

	logs[0]->scale(0.5);
	logs[0]->translate(glm::vec3(-7,1,0));
	logs[1]->scale(0.40);
	logs[1]->translate(glm::vec3(-3,1.5,15));
	logs[2]->scale(0.35);
	logs[2]->translate(glm::vec3(14,2.15,5));

	// Demo models
	for(auto& model : demoModels)
	{
		model->fragmentSettings.noiseEffect = Model::NoiseType::BLACK_WHITE;
		model->fragmentSettings.persistence = 1;
		model->fragmentSettings.ringFrequency = 80;
		model->fragmentSettings.octaveCount = 1;
		model->fragmentSettings.octaveStart = 0;
		model->fragmentSettings.phaseSpeed = 1.4;
		model->fragmentSettings.minFrequency = 50;
		model->fragmentSettings.maxFrequency = 200;
		model->fragmentSettings.waveCenters = 20;
	}
		demoModels[0]->translate(glm::vec3(-4, 5, 0));
		demoModels[1]->translate(glm::vec3(-5, 12, 0));
		demoModels[2]->translate(glm::vec3(0, 30, 0));
		demoModels[3]->translate(glm::vec3(5, 15, -0.1));

	for(auto& model : models)
		model->update();
}

void Renderer::run()
{
	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.2f, 0.3f, 0.3f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processWindowInput();

		shader->use();
		shader->setUniformMatrix4fv("view", camera.getViewMatrix());
		shader->setUniformMatrix4fv("perspective", perspective);
		shader->setUniform1f("time", currentFrame);
		shader->setUniform3fv("toCamera", camera.getPosition());

		for (auto& model : models)
		{
			model->rotate(rotate);
			model->scale(scale);
			model->update();
			model->draw(*shader);
		}

		glUseProgram(0);

		rotate = glm::vec3(0.0f);
		scale = 1;
		
		showGui();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

/*
 * Display the ImGui and handle its events.
 */
void Renderer::showGui()
{
	auto HelpMarker = [](const char* desc) -> void
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	};
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Fragment Shader Settings");

	if (ImGui::CollapsingHeader("Grass/Terrain", ImGuiTreeNodeFlags_None))
	{
		Model::FragmentSettings& fs = terrain->fragmentSettings;
		ImGui::SliderFloat("Persistence###grp", &fs.persistence, 0.1, 1.0);
		ImGui::SameLine(); HelpMarker("The ith amplitude is persistence^i.");
		ImGui::SliderInt("Octaves###gro", &fs.octaveCount, 1, 16);
		ImGui::SameLine(); HelpMarker("The more octaves are added the smoother the noise will be.");
		ImGui::SliderInt("Octave Start###gros", &fs.octaveStart, 0, fs.octaveCount-1);
	}

	for (unsigned int i = 0; i < logs.size(); i++)
	{
		std::string header = "Wood " + std::to_string(i+1);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_None))
		{
			Model::FragmentSettings& fs = logs[i]->fragmentSettings;
			std::string persistence = "Persistence###wp " + std::to_string(i);
			std::string ringFreq = "Ring Frequency###wf " + std::to_string(i);
			std::string octaves = "Octaves###woc" + std::to_string(i);
			std::string octavesStart = "Octaves###wocs" + std::to_string(i);


			ImGui::SliderFloat(persistence.c_str(), &fs.persistence, 0.1, 1.0);
			ImGui::SameLine(); HelpMarker("The ith amplitude is persistence^i.");
			ImGui::SliderFloat(ringFreq.c_str(), &fs.ringFrequency, 0.1, 100.0);

			ImGui::SliderInt(octaves.c_str(), &fs.octaveCount, 1, 16);
			ImGui::SameLine(); HelpMarker("The more octaves are added the smoother the noise will be.");
			ImGui::SliderInt(octavesStart.c_str(), &fs.octaveStart, 0, fs.octaveCount-1);
		}
	}

	if (ImGui::CollapsingHeader("Waves", ImGuiTreeNodeFlags_None))
	{
		Model::FragmentSettings& fs = water->fragmentSettings;
		ImGui::SliderInt("Wave Centers", &fs.waveCenters, 0, 20);
		ImGui::SliderFloat("Wave Speed", &fs.phaseSpeed, 0, 3.0);
		ImGui::SliderFloat("Min Frequency", &fs.minFrequency, 1, fs.maxFrequency);
		ImGui::SliderFloat("Max Frequency", &fs.maxFrequency, 0.01, 500);
	}

	if (ImGui::CollapsingHeader("Demo Models", ImGuiTreeNodeFlags_None))
	{
		const char* noiseNames[Model::NoiseType::COUNT] = {
			"Grass/Terrain",
			"Wood",
			"Water",
			"Black/White",
			"None"};
		Model::FragmentSettings& fs = demoModels[0]->fragmentSettings;
		//int noiseEffect = fs.noiseEffect;
		ImGui::SliderInt("Texture", reinterpret_cast<int*>(&fs.noiseEffect), 0, Model::NoiseType::COUNT - 1, noiseNames[fs.noiseEffect]);

		ImGui::SliderFloat("Persistence###demop", &fs.persistence, 0.1, 1.0);
		ImGui::SameLine(); HelpMarker("The ith amplitude is persistence^i.");

		ImGui::SliderInt("Octaves###demoo", &fs.octaveCount, 1, 16);
		ImGui::SameLine(); HelpMarker("The more octaves are added the smoother the noise will be.");

		ImGui::SliderInt("Octave Start###demoos", &fs.octaveStart, 0, fs.octaveCount-1);

		ImGui::SliderFloat("Ring Frequency###demorf", &fs.ringFrequency, 0.1, 100.0);

		ImGui::SliderInt("Wave Centers###demowc", &fs.waveCenters, 0, 20);
		ImGui::SliderFloat("Wave Speed###demows", &fs.phaseSpeed, 0, 3.0);
		ImGui::SliderFloat("Min Frequency###demowmax", &fs.minFrequency, 1, fs.maxFrequency);
		ImGui::SliderFloat("Max Frequency###demomin", &fs.maxFrequency, 0.01, 500);

		for(auto& model : demoModels)
		{
			Model::FragmentSettings& demoFs = model->fragmentSettings;
			demoFs.noiseEffect = fs.noiseEffect;
			demoFs.persistence = fs.persistence;
			demoFs.ringFrequency = fs.ringFrequency;
			demoFs.octaveCount = fs.octaveCount;
			demoFs.octaveStart = fs.octaveStart;
			demoFs.waveCenters = fs.waveCenters;
			demoFs.phaseSpeed = fs.phaseSpeed;
			demoFs.minFrequency = fs.minFrequency;
			demoFs.maxFrequency = fs.maxFrequency;
		}
	}
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/*
 * This method typically runs faster than handling a key callback.
 * So controls like movements should be placed in here.
 */
void Renderer::processWindowInput()
{
	// Camera Movement
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.processKeyboard(Camera::Movement::FORWARD, deltaTime);
	}

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.processKeyboard(Camera::Movement::BACKWARD, deltaTime);
	}

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.processKeyboard(Camera::Movement::RIGHT, deltaTime);
	}

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.processKeyboard(Camera::Movement::LEFT, deltaTime);
	}

	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.processKeyboard(Camera::Movement::UP, deltaTime);
	}

	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.processKeyboard(Camera::Movement::DOWN, deltaTime);
	}
}

/*
 * Handle keyboard inputs that don't require frequent repeated actions,
 * ex closing window, selecting model etc.
 */ 
void Renderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
	
	if (action == GLFW_PRESS)
	{
		switch(key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;
			case GLFW_KEY_SPACE:
				renderer->showCursor = !renderer->showCursor;
				if (renderer->showCursor)
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				else
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				break;

			// Select model
			case GLFW_KEY_1:
			case GLFW_KEY_2:
			case GLFW_KEY_3:
			case GLFW_KEY_4:
			case GLFW_KEY_5:
			case GLFW_KEY_6:
			case GLFW_KEY_7:
			case GLFW_KEY_8:
			case GLFW_KEY_9:
				//renderer->modelIndex = key - GLFW_KEY_1;
				break;
		}
	}
}

void Renderer::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
	// if cursor is showing we want to control ImGui, so don't move camera around.
	if (!renderer->showCursor)
	{

		if (renderer->firstMouse)
		{
			renderer->lastX = xpos;
			renderer->lastY = ypos;
			renderer->firstMouse = false;
		}

		float xoffset = xpos - renderer->lastX;
		float yoffset = renderer->lastY - ypos; // reversed since y-coordinates go from bottom to top

		renderer->lastX = xpos;
		renderer->lastY = ypos;

		renderer->camera.processMouseMovement(xoffset, yoffset);
	}
}
