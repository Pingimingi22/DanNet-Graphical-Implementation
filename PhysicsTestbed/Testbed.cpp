#include "Testbed.h"
#include "GLFWCallbacks.h"

glm::mat4 Testbed::GetCameraTransform() const
{
	return glm::ortho(
		-aspectRatio * cameraHeight / 2.0f + cameraCentre.x, 
		aspectRatio * cameraHeight / 2.0f + cameraCentre.x,
		-cameraHeight / 2.0f + cameraCentre.y,
		cameraHeight / 2.0f + cameraCentre.y,
		-1.0f, 1.0f);
}

Testbed::Testbed()
{
	if (!glfwInit())
	{
		return;
	}
	//Can choose resolution here.
	window = glfwCreateWindow(1280, 720, "Physics Testbed", nullptr, nullptr);

	//This is the somewhat hacky oldschool way of making callbacks work without everything having to be global. Look
	//at the way the function callbacks work to get an idea of what's going on.
	glfwSetWindowUserPointer(window, (void*)this);	

	//These functions will be able to reference the testbed object via the user pointer.
	glfwSetWindowSizeCallback(window, WindowResizeCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, MouseWheelCallback);

	if (!window)
	{
		//If the window failed to create for some reason, abandon ship.
		glfwTerminate();
		return;
	}

	//We have to tell glfw to use the OpenGL context from the window.
	glfwMakeContextCurrent(window);

	//This is where GLAD gets set up. After this point we can use openGL functions.
	if (!gladLoadGL())
	{
		return;
	}
	glfwSwapInterval(1);

	simpleShader = ShaderProgram("Simple.vsd", "Simple.fsd");
	simpleShader.UseShader();

	lines.Initialise();

	glClearColor(0, 0, 0, 1);

	grid.Initialise();
	for (float i = (float)-gridLimits; i <= (float)gridLimits; i++)
	{
		glm::vec3 colour = (i == 0) ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.3f, 0.3f, 0.3f);
		grid.DrawLineSegment({ i, -gridLimits }, { i, gridLimits }, colour);
		grid.DrawLineSegment({ -gridLimits, i }, { gridLimits, i }, colour);
	}
	grid.DrawLineSegment({ 0,0 }, { 1, 0 }, { 1, 0, 0 });
	grid.DrawLineSegment({ 0,0 }, { 0, 1 }, { 0, 1, 0 });
	grid.Compile();
}

Testbed::~Testbed()
{
	glfwTerminate();
}

void Testbed::Update()
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	aspectRatio = width / (float)height;

	time += deltaTime;

	glm::mat4 deprojection = glm::inverse(GetCameraTransform());
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	cursorX = (cursorX / width) * 2.0 - 1.0;
	cursorY = -((cursorY / height) * 2.0 - 1.0);

	glm::vec4 mousePosNDC(float(cursorX), float(cursorY), 0, 1);

	glm::vec4 mousePosWorld = deprojection * mousePosNDC;

	cursorPos.x = mousePosWorld.x;
	cursorPos.y = mousePosWorld.y;

	// ============================== Temporarily removing this because my little demo needs to let the player move the character with these keys. ============================== // 

	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	//{
	//	cameraCentre.x -= cameraSpeed * deltaTime * cameraHeight;
	//}
	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	//{
	//	cameraCentre.x += cameraSpeed * deltaTime * cameraHeight;
	//}
	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	//{
	//	cameraCentre.y += cameraSpeed * deltaTime * cameraHeight;
	//}
	//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	//{
	//	cameraCentre.y -= cameraSpeed * deltaTime * cameraHeight;
	//}

	leftButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	rightButtonDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

void Testbed::Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glm::mat4 orthoMat = GetCameraTransform();
	simpleShader.SetUniform("vpMatrix", orthoMat);
	grid.Draw();	//Grid lines don't change so we just draw them.
	lines.UpdateFrame();	//Other lines potentially change every frame, so we have to compile/draw/clear them.
	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool Testbed::IsRunning() const
{
	return !glfwWindowShouldClose(window);
}

void Testbed::OnMouseClick(int mouseButton)
{
	//Override this function if you need to respond to mouse clicks.
}

void Testbed::OnMouseRelease(int mouseButton)
{
	//Override this function if you need to respond to mouse button releases.
}

void Testbed::Zoom(float zoomFactor)
{
	cameraHeight /= zoomFactor;
}
