file := Src/main.cpp Src/application.cpp Src/window.cpp Src/Renderer/renderer.cpp Src/VulkanFramework/instance.cpp Src/VulkanFramework/logicalDevice.cpp Src/VulkanFramework/physicalDevice.cpp Src/VulkanFramework/swapchain.cpp

cFlags := -std=c++17 -O2
ldFlags := -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

application: $(file) $(shaders)
	g++ $(cFlags) -o application $(file) $(ldFlags)

clean:
	rm -f application
