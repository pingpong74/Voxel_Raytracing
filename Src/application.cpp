#include "application.hpp"
#include "../includes/Renderer/renderer.hpp"

Application::Application() : window(), renderer(window.handle, window.width, window.height) {

}
////////////////////////////////////////// IMPROVE SRNCRONIZATION THIS SHIT IS ASSS IMPROVE THIS PLEASEE REMBER TO IMPROVE THIS HAHAHAHAHHAHAHAHAHAHH H HH FU FENUFNFE JFE FUCKKKKKKKKKKKKKKKKKKKKKKKKKKKK ///////////////////////////////////////////////////
/*void Application::main_loop() {
    FrameData framedata[FRAME_IN_FLIGHT];

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenceCreateInfo.pNext = nullptr;

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.flags = 0;
    semaphoreCreateInfo.pNext = nullptr;

    for(int i = 0; i < FRAME_IN_FLIGHT; i++) {
        VK_CHECK(vkCreateFence(logicalDevice.handle, &fenceCreateInfo, nullptr, &framedata[i].inFlightFence), "Failed to create fence");

        VK_CHECK(vkCreateSemaphore(logicalDevice.handle, &semaphoreCreateInfo, nullptr, &framedata[i].imageSemaphore), "Failed to create Semaphore");
        VK_CHECK(vkCreateSemaphore(logicalDevice.handle, &semaphoreCreateInfo, nullptr, &framedata[i].renderFinishSemaphore), "Failed to create Semaphore");

        framedata[i].commandBuffer = logicalDevice.createCommandBuffer(graphicsPool);
    }

	float lastFrame = 0;
	uint32_t frameIndex = 0;

	VkResult resize;

	while(!glfwWindowShouldClose(window)){
		float deltaTime = glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();

		vkWaitForFences(logicalDevice.handle, 1, &framedata[frameIndex].inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(logicalDevice.handle, 1, &framedata[frameIndex].inFlightFence);

		framedata[frameIndex].commandBuffer.reset();

		uint32_t imageIndex;
    	resize = vkAcquireNextImageKHR(logicalDevice.handle, swapchain.handle, UINT64_MAX, framedata[frameIndex].imageSemaphore, VK_NULL_HANDLE, &imageIndex);

		raytracer.drawFrame(framedata[frameIndex].commandBuffer, swapchain.swapchainImages[imageIndex], deltaTime);

		VkPipelineStageFlags stageFlags[] = {VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &framedata[frameIndex].renderFinishSemaphore;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &framedata[frameIndex].imageSemaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &framedata[frameIndex].commandBuffer.handle;
		submitInfo.pWaitDstStageMask = stageFlags;

		VK_CHECK(vkQueueSubmit(logicalDevice.graphicsQueue, 1, &submitInfo, framedata[frameIndex].inFlightFence), "Failed to submit to queue");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain.handle;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &framedata[frameIndex].renderFinishSemaphore;

		resize = vkQueuePresentKHR(logicalDevice.presentationQueue, &presentInfo);

		cout << "\rFPS : " << (1 / deltaTime);

		frameIndex = frameIndex % FRAME_IN_FLIGHT;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	vkDeviceWaitIdle(logicalDevice.handle);

	for(int i = 0; i < FRAME_IN_FLIGHT; i++) {
        vkDestroyFence(logicalDevice.handle, framedata[i].inFlightFence, nullptr);
	    vkDestroySemaphore(logicalDevice.handle, framedata[i].imageSemaphore, nullptr);
	    vkDestroySemaphore(logicalDevice.handle, framedata[i].renderFinishSemaphore, nullptr);
		logicalDevice.destroyCommandBuffer(framedata[i].commandBuffer);
	}
	}*/
