#include "camera.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>

#include <iostream>

Camera::Camera(GLFWwindow* window) {
    lastX = (double)1920 * 0.5;
	lastY = (double)1080 * 0.5;

	firstMouse = true;

	yaw = -90.0f;
	pitch = 0.0f;
	roll = 0.0f;

	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	this->window = window;

	camCons = {
	    glm::inverse(glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp)),
		glm::inverse(glm::perspective(glm::radians(45.0f), 800.0f/ 600.0f, 0.1f, 100.0f))
	};
}

void Camera::updateCamera(float deltaTime) {
	cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	takeInput(deltaTime);
	camCons.inverseView = glm::inverse(glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp));
}

void Camera::mouseCallback(double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	const float senstivity = 0.1f;
	float x = -senstivity * (xpos - lastX);
	float y = -senstivity * (ypos - lastY);
	lastX = xpos;
	lastY = ypos;

	pitch += y;
	yaw += x;

	if (pitch > 89.5f) pitch = 89.5f;
	if (pitch < -89.5f) pitch = -89.5f;

	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void Camera::frameBufferResize(int width, int height) {
    camCons.inverseProjection = glm::inverse(glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f));
    std::cout << "HIWVVVVVVVVEJKKNNNNN " << std::endl;
}

void Camera::takeInput(float deltaTime) {
    float cameraSpeed = 10.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W))  cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S))  cameraPos -= cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_D)) cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A)) cameraPos -= cameraSpeed * cameraRight;
}
