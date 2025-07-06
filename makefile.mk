file := Src/main.cpp Src/application.cpp Src/window.cpp Lib/Renderer/renderer.cpp Lib/VulkanFramework/instance.cpp Lib/VulkanFramework/logicalDevice.cpp Lib/VulkanFramework/physicalDevice.cpp Lib/VulkanFramework/swapchain.cpp Lib/VulkanFramework/buffer.cpp Lib/VulkanFramework/descriptor.cpp Lib/VulkanFramework/commandPool.cpp Lib/VulkanFramework/pipelines.cpp

cFlags := -std=c++17 -O2
ldFlags := -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

application: $(file) $(shaders)
	g++ $(cFlags) -o application $(file) $(ldFlags)

clean:
	rm -f application
