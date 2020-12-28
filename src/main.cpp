#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>

#include "Renderer.h"

int main(int argc, char *argv[])
{
	int seed = 0;
	if (argc >= 2)
	{
		try
		{
			seed = std::stoi(argv[1]);
			std::cout << seed;
		}
		catch (std::invalid_argument& ia)
		{
			std::cerr << "Usage: ./myapp <seed>\n";
			return -1;
		}
	}
	{
		Renderer renderer(seed);
		renderer.run();
	}
	// Need to terminate GLFW context after all OpenGL objects are deleted.
	// Otherwise, a seg fault will occur.
	glfwTerminate();
}
