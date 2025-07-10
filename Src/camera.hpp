#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>

struct CameraConstants {
    glm::mat4 inverseView;
    glm::mat4 inverseProjection;
};

class Camera {

	public:
	Camera(GLFWwindow* window);

	~Camera() = default;

	void updateCamera(float deltaTime);

	void mouseCallback(double xpos, double ypos);
	void frameBufferResize(int width, int height);

	inline CameraConstants getCamMats() {return camCons; }

	private:

	CameraConstants camCons;

	GLFWwindow* window;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 cameraRight;

    double lastX;
    double lastY;

    float yaw;
    float pitch;
    float roll;

    glm::vec3 direction;
    bool firstMouse;

    void takeInput(float deltaTime);
};
