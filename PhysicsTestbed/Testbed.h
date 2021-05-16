#pragma once
#include "Graphics.h"
#include "Maths.h"
#include "LineRenderer.h"
#include "ShaderProgram.h"


class Testbed
{
	float cameraHeight = 10.0f;
	glm::vec2 cameraCentre = { 0.0f, 0.0f };
	
protected:
	GLFWwindow* window = nullptr;

private:
	ShaderProgram simpleShader;
	float aspectRatio = 16.0f / 9.0f;

	LineRenderer grid;

	const float cameraSpeed = 0.8f;

	const int gridLimits = 10;

	glm::mat4 GetCameraTransform() const;


protected:
	glm::vec2 cursorPos = { 0.0f,0.0f };
	bool leftButtonDown = false;
	bool rightButtonDown = false;
	float time = 0.0f;
	LineRenderer lines;

public:
	Testbed();
	~Testbed();

	void Update();
	void Render();

	bool IsRunning() const;

	void Shutdown() const;

	//1 for left, 2 for right, 3 for middle. Other button codes will get sent for other buttons but I'm not sure how they'd map to a particular mouse.
	virtual void OnMouseClick(int mouseButton);
	virtual void OnMouseRelease(int mouseButton);

	void Zoom(float zoomFactor);

	const float deltaTime = 0.0166667f;	//Delta time should be constant for physics simulations.
};