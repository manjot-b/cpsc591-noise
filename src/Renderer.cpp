#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <filesystem>
#include <cstdlib>

#include "Renderer.h"

Renderer::Renderer() :
	rotate(0), scale(1), camera(glm::vec3(0,5,12)),
	firstMouse(true), lastX(width / 2.0f), lastY(height / 2.0f),
	shiftPressed(false), deltaTime(0.0f), lastFrame(0.0f)
{
	initWindow();
	shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
	shader->link();
	loadModels();	
	setupModels();
	
	perspective = glm::perspective(glm::radians(45.0f), float(width)/height, 0.1f, 100.0f);
	shader->use();
	shader->setUniformMatrix4fv("perspective", perspective);
	shader->setUniformMatrix4fv("view", camera.getViewMatrix());

	glm::vec3 lightPos = glm::vec3(5.0, 25.0, -20.0);
	shader->setUniform3fv("lightPos", lightPos);

	int perm[256];
	for (unsigned int i = 0; i < 256; i++)
		perm[i] = i;
	shuffle(perm, 110);
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

void Renderer::loadModels()
{
	namespace fs = std::filesystem;
	const std::string dir = "models/";
	const std::string logPath = dir + "log.obj";
	const std::string terrainPath = dir + "terrain-3.obj";
	const std::string waterPath = dir + "water.obj";

	unsigned int logCount = 3;
	for (unsigned int i = 0; i < logCount; i++)
	{
		std::cout << "Loading " << logPath << "..." << std::flush;
		logs.push_back(std::make_unique<Model>(logPath));
		std::cout << "Done!\n"; 
	}
	std::cout << "Loading " << waterPath << "..." << std::flush;
	water = std::make_unique<Model>(waterPath);
	std::cout << "Done!\n"; 

	std::cout << "Loading " << terrainPath << "..." << std::flush;
	terrain = std::make_unique<Model>(terrainPath);
	std::cout << "Done!\n"; 

	models.push_back(water);
	models.push_back(terrain);
	for (unsigned int i = 0; i < logCount; i++)
	{
		models.push_back(logs[i]);
	}
}

void Renderer::setupModels()
{
	// Water
	water->fragmentSettings.noiseEffect = Model::NoiseType::WATER;
	water->fragmentSettings.phaseSpeed = 1.8;
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
		log->fragmentSettings.octaveCount = 3;
		log->fragmentSettings.octaveStart = 0;
	}

	logs[0]->scale(0.5);
	logs[0]->translate(glm::vec3(-7,1,0));
	logs[1]->scale(0.40);
	logs[1]->translate(glm::vec3(-3,1.5,15));
	logs[2]->scale(0.35);
	logs[2]->translate(glm::vec3(14,2.15,5));

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

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

/*
 * This method typically runs faster than handling a key callback.
 * So controls like movements should be placed in here.
 */
void Renderer::processWindowInput()
{
	float rotationSpeed = glm::radians(135.0f) * deltaTime;
	float scaleSpeed = 1.0f + 1.0f * deltaTime;
	shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

	// Rotations
	if (!shiftPressed)
	{
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			rotate.x -= rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			rotate.x += rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			rotate.y += rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			rotate.y -= rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			rotate.z -= rotationSpeed;
		}

		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			rotate.z += rotationSpeed;
		}
	}

	// Camera Movement
	if (shiftPressed)
	{
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

	// Scaling
	if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		scale *= scaleSpeed;
	}
	if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	{
		scale /= scaleSpeed;
	}

}

/*
 * Handle keyboard inputs that don't require frequent repeated actions,
 * ex closing window, selecting model etc.
 */ 
void Renderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
	
	if (action == GLFW_PRESS)
	{
		switch(key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
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
