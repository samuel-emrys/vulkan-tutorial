#include <vulkan/vulkan.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

class HelloTriangleApplication {
  public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

  private:
    void initVulkan() {}

    void mainLoop() {}

    void cleanup() {}
};

auto main() -> int {
    HelloTriangleApplication vApp;

    try {
        vApp.run();
    } catch (const std::exception& vE) {
        std::cerr << vE.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
