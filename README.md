# Compiling
## Linux

Use `git` to clone the submodules first.

	git submodule update --init --recursive --depth 1

**TO-DO:** Migrate this project over to using CMake so that the following step is not required.

Build both **assimp** and **glfw** by replacing the project name with the actual name of the dependancy.

	cd dep/<project>
	mkdir build
	cd build
	cmake ..
	make -jN

Come back to the root directory and enter  `make -jN`.

## Windows
I am not sure how to get this to run on Windows but I can give some suggestions based on how the project is structured. Read about the required libraries in the *Compiling and Running on Linux* section above.

- **src/** contains all the `.cpp` and personal `.h` files.
- **inc/** contains all the vendor `.h` files.
- **lib/** should contain the dynamic library files which are not already include in your `PATH`.
- **rsc/** contains the assets (models, shaders and textures) which will be symbollically linked in the **bin/** folder when compiling. This step must happen, otherwise the files will need to be copied into the **bin/** directory because the executable accesses these assuming they are symbolically linked relative to the executable.
- **obj/** an intermediary build folder for `.o` files. Not required but keeps **bin/** clean.
- **bin/** and **lib/** are for the outputs of compiling. You will find the executable in **bin/**.

# Running
Head into the **bin/** directory and enter `./myapp`.

# Controls
- Camera Movement *W, A, S, D, E, Q*.
- Camera Direction *MOVE CURSOR*.
- Toggle camera/gui focus *SPACE*.
