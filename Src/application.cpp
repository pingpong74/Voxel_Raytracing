#include "application.hpp"
#include "Renderer/renderer.hpp"

Application::Application() : window(), renderer(window.handle, window.width, window.height) {

}
