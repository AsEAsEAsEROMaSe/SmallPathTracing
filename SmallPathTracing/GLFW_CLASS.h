#pragma once
#include "Camera.h"
#include "Sphere.h"
#include "VEC3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>	
#include <thread>


float moveSpeed = 2.5f;//0.5
float mouseSensitivity = 0.10f;//0.05
bool keyPressed = false;
double xpos, ypos;
bool cursor_enter_this_window = false;
int WIDTH_WINDOW;
int HEIGHT_WINDOW;
 //
GLuint tex_output;
int work_grp_cnt[3];
int work_grp_size[3];
int work_grp_inv;
GLuint ray_shader;
GLuint ray_program;
GLuint VBO, VAO;

void cursor_enter_callback(GLFWwindow* window, int entered)
{
	if (entered)
	{
		// The cursor entered the content area of the window
		cursor_enter_this_window = true;
	}
	else
	{
		// The cursor left the content area of the window
		cursor_enter_this_window = false;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void CenterTheWindow(GLFWwindow* window)
{
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwSetWindowPos(window, (mode->width - WIDTH_WINDOW) / 2, (mode->height - HEIGHT_WINDOW) / 2);
}

void OpenGLINIT(std::vector<GLuint> pixels)
{
	// dimensions of the image
	
	glGenTextures(1, &tex_output);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH_WINDOW, HEIGHT_WINDOW, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	glGenerateMipmap(GL_TEXTURE_2D);


	std::string shader = "\
		#version 330 core\n\
		out vec4 FragColor;\n\
	in vec3 ourColor;\n\
	in vec2 TexCoord;\n\
	uniform sampler2D ourTexture;\n\
	void main()\n\
	{\n\
		FragColor = texture(ourTexture, TexCoord);\n\
	}\n\
	";
	auto the_ray_shader_string = shader.c_str();

	ray_shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(ray_shader, 1, &the_ray_shader_string, NULL);
	glCompileShader(ray_shader);
	// check for compilation errors as per normal here
	// 
	//shader vertex
	const char* vertexShaderSource = "\
		#version 330 core\n \
		layout(location = 0) in vec3 aPos;\n\
	layout(location = 1) in vec3 aColor;\n\
	layout(location = 2) in vec2 aTexCoord;\n\
\n\
	out vec3 ourColor;\n\
	out vec2 TexCoord;\n\
\n\
	void main()\n\
	{\n\
		gl_Position = vec4(aPos, 1.0);\n\
		ourColor = aColor;\n\
		TexCoord = aTexCoord;\n\
	}";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// link shaders
	ray_program = glCreateProgram();
	glAttachShader(ray_program, vertexShader);
	glAttachShader(ray_program, ray_shader);
	glLinkProgram(ray_program);
	// check for linking errors
	glGetProgramiv(ray_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ray_program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	///////

	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

}

void OpenGLUpdate()
{
	//{ // launch compute shaders!
	//	glUseProgram(ray_program);
	//	glDispatchCompute((GLuint)WIDTH_WINDOW, (GLuint)HEIGHT_WINDOW, 1);
	//}

	// make sure writing to image has finished before read
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	{ // normal drawing pass
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(ray_program);
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_output);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

GLFWwindow* GLFW_INIT(int width, int height)
{
	WIDTH_WINDOW = width;
	HEIGHT_WINDOW = height;

	GLFWwindow* window;
	glfwInit();
	window = glfwCreateWindow(WIDTH_WINDOW, HEIGHT_WINDOW, "PathTracer", 0, 0);
	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, 0, 0);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Клавіші залипають поки не будуть опитані це для того щоб не прогавити подію натиску поки не було обновлення
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetCursorPos(window, WIDTH_WINDOW /2, HEIGHT_WINDOW /2);
	glfwGetCursorPos(window, &xpos, &ypos);
	
	glfwSwapInterval(0);
	glfwMakeContextCurrent(window);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glewInit();

	return window;
}

Camera* CAMERA_INIT()
{
	Camera* camera = new Camera({ -1,-1,-1 }, { 1,1,1 }, 90);
	return camera;
}

void key_update(float secondsElapsed, GLFWwindow* window, Camera* camera)
{
	keyPressed = false;
	//Camera Movement
	if (glfwGetKey(window, 'W') == GLFW_PRESS) {
		camera->offsetPosition(secondsElapsed * moveSpeed * camera->forward);
		keyPressed = true;
	}
	if (glfwGetKey(window, 'S') == GLFW_PRESS) {
		camera->offsetPosition(secondsElapsed * moveSpeed * -camera->forward);
		keyPressed = true;
	}
	if (glfwGetKey(window, 'A') == GLFW_PRESS) {
		camera->offsetPosition(secondsElapsed * moveSpeed * -camera->right);
		keyPressed = true;
	}
	if (glfwGetKey(window, 'D') == GLFW_PRESS) {
		camera->offsetPosition(secondsElapsed * moveSpeed * camera->right);
		keyPressed = true;
	}

	//Mouse Handling
	camera->isMoving = false;
	if (cursor_enter_this_window)
	{
		double new_xpos, new_ypos;
		glfwGetCursorPos(window, &new_xpos, &new_ypos);
		glfwSetCursorPos(window, WIDTH_WINDOW / 2, HEIGHT_WINDOW / 2);
		double delta_xpos = xpos - new_xpos;
		double delta_ypos = ypos - new_ypos;
		camera->offsetOrientation(mouseSensitivity * delta_xpos, mouseSensitivity * delta_ypos);
		camera->isMoving = true;
	}
}

void scene_update(float secondsElapsed, GLFWwindow* window, Camera* camera, std::vector<GLuint> pixels)
{
	glClearColor(0.001f, 0.001f, 0.001f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDrawPixels(WIDTH_WINDOW, HEIGHT_WINDOW, GL_RGBA, GL_RGB8UI, pixels.data());
	//glFlush();
	OpenGLUpdate();
}

void GLFW_LOOP(
	GLFWwindow* window, 
	Camera* camera, 
	void(*render_function)(const std::vector<Sphere>& spheres, Vec3f* pixel, unsigned width, unsigned height), 
	std::vector<GLuint> pixels,
	std::vector<Sphere>& spheres, 
	Vec3f* pixel)
{
	std::atomic_bool state_close;
	std::thread thread([&]()
	{
		while (true)
		{
			if (state_close)
			{
				render_function(spheres, pixel, WIDTH_WINDOW, HEIGHT_WINDOW);

				for (long long i = 0; i < HEIGHT_WINDOW * WIDTH_WINDOW; i++)
				{
					pixels[i + 0] = (std::min(float(1), pixel[i].x) * 255);
					pixels[i + 1] = (std::min(float(1), pixel[i].y) * 255);
					pixels[i + 2] = (std::min(float(1), pixel[i].z) * 255);
				}
			}
		}
	});


	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		double presentTime = glfwGetTime();
		key_update((float)(presentTime - lastTime), window, camera);
		scene_update((float)(presentTime - lastTime), window, camera, pixels);
		lastTime = presentTime;
		glfwSwapBuffers(window);
	}
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwTerminate();
	state_close = false;
	thread.~thread();
}

std::vector<GLuint> CREATE_BUFFER(int width, int height)
{
	std::vector<GLuint>pixels;
	pixels.resize(width * height * 3);
	for (long long i = 0; i < width * height * 3; i++)
	{
		pixels[i] = 255;
	}
	return pixels;
}

void GLFW_WORK(
	std::vector<GLuint> pixels,
	int width, 
	int height, 
	void(*render_function)(const std::vector<Sphere>& spheres, Vec3f* pixel, unsigned width, unsigned height), 
	std::vector<Sphere>& spheres, 
	Vec3f* pixel)
{
	GLFWwindow* window = GLFW_INIT(width, height); 
	OpenGLINIT(pixels);
	Camera* camera = CAMERA_INIT();
	GLFW_LOOP(window, camera, render_function, pixels, spheres, pixel);
}