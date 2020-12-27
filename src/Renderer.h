#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#include <memory>

#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"

class Renderer
{
	public:
		Renderer();
		~Renderer();
		void run();

	private:
		GLFWwindow* window;
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Model> terrain;
		std::shared_ptr<Model> water;
		std::vector<std::shared_ptr<Model>> logs;
		std::vector<std::shared_ptr<Model>> models;
		std::vector<std::shared_ptr<Model>> demoModels;
		
		const unsigned int height = 800;
		const unsigned int width = 800;
		bool showCursor;

		glm::vec3 rotate;
		float scale;
		Camera camera;
		glm::mat4 perspective;

		bool firstMouse;
		float lastX;
		float lastY;
		bool shiftPressed;

		float deltaTime;
		float lastFrame;

		void shuffle(int perm[256], int seed);
		void initWindow();
		void initImGui();
		void loadModels();
		void loadModel(const std::string path, std::shared_ptr<Model>& model);
		void setupModels();
		void showGui();
		void processWindowInput();
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
};
