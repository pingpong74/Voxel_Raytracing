#include "application.hpp"
#include "Renderer/renderer.hpp"
#include "Scene/scene.hpp"
#include <GLFW/glfw3.h>

Application::Application()
: window(this),
instance(window.handle),
logicalDevice(&instance),
swapchain(&logicalDevice, width, height, instance.surface),
graphicsPool(&logicalDevice, logicalDevice.graphicsQueue.familyIndex),
transferPool(&logicalDevice, logicalDevice.transferQueue.familyIndex),
computePool(&logicalDevice, logicalDevice.computeQueue.familyIndex),
renderer(&logicalDevice, &transferPool, &graphicsPool, &computePool, &swapchain),
scene(&logicalDevice, &computePool, window.handle) {

    renderer.setScene(&scene);

}

void Application::frameBufferResize(int width, int height) {
    swapchain.recreateSwapchain(width, height);
    renderer.frameBufferResize(width, height);
    scene.frameBufferResize(width, height);
}

void Application::mouseCallBack(double xpos, double ypos) {
    scene.mouseCallBack(xpos, ypos);
}

void Application::run() {

    double lastframe = 0;
    double deltaTime = 0;

    int i = 0;

    while(!window.shouldClose()) {

        //i++;

        deltaTime = glfwGetTime() - lastframe;
        lastframe = glfwGetTime();

        scene.update(deltaTime);
        renderer.drawFrame();

        //std::cout << deltaTime << std::endl;

        window.poll();

        //if(i > 5) break;
    }
}
