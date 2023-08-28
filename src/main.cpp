#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

const uint32_t gWIDTH = 800;
const uint32_t gHEIGHT = 800;
class HelloTriangleApplication {
  public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

  private:
    GLFWwindow* mWindow;
    VkInstance mInstance;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        mWindow = glfwCreateWindow(gWIDTH, gHEIGHT, "Vulkan", nullptr, nullptr);
    }

    void createInstance() {
        VkApplicationInfo vAppInfo{};
        vAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        vAppInfo.pApplicationName = "Hello Triangle";
        vAppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        vAppInfo.pEngineName = "No Engine";
        vAppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        vAppInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo vCreateInfo{};
        vCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        vCreateInfo.pApplicationInfo = &vAppInfo;

        uint32_t vGlfwExtensionCount = 0;
        const char** pvGlfwExtensions = nullptr;

        pvGlfwExtensions = glfwGetRequiredInstanceExtensions(&vGlfwExtensionCount);
        vCreateInfo.enabledExtensionCount = vGlfwExtensionCount;
        vCreateInfo.ppEnabledExtensionNames = pvGlfwExtensions;
        vCreateInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&vCreateInfo, nullptr, &mInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

    }

    void initVulkan() {
        createInstance();
    }

    void mainLoop() {
        while (glfwWindowShouldClose(mWindow) == 0) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroyInstance(mInstance, nullptr);
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }
};

auto main() -> int {
    HelloTriangleApplication vApp{};

    try {
        vApp.run();
    } catch (const std::exception& vError) {
        std::cerr << vError.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
