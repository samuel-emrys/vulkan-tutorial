#include <cstdint>
#include <cstring>
#include <vulkan/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

const uint32_t gWIDTH = 800;
const uint32_t gHEIGHT = 800;
const std::vector<const char*> gVALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool gENABLE_VALIDATION_LAYERS = false;
#else
const bool gENABLE_VALIDATION_LAYERS = true;
#endif


auto createDebugUtilsMessengerEXT(VkInstance prInstance,
                                  const VkDebugUtilsMessengerCreateInfoEXT* prCreateInfo,
                                  const VkAllocationCallbacks* prAllocator,
                                  VkDebugUtilsMessengerEXT* prDebugMessenger)
    -> VkResult {
    auto pvFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        prInstance,
        "vkCreateDebugUtilsMessengerEXT");
    if (pvFunc != nullptr) {
        return pvFunc(prInstance, prCreateInfo, prAllocator, prDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

auto destroyDebugUtilsMessengerEXT(VkInstance prInstance,
                                   VkDebugUtilsMessengerEXT prDebugMessenger,
                                   const VkAllocationCallbacks* prAllocator) {
    auto pvFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        prInstance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (pvFunc != nullptr) {
        std::cerr << "destroy instance not null" << std::endl;
        pvFunc(prInstance, prDebugMessenger, prAllocator);
    } else {
        std::cerr << "vkDestroyDebugUtilsMessengerExt doesn't exist!" << std::endl;
    }
}

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
    VkDebugUtilsMessengerEXT mDebugMessenger;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        mWindow = glfwCreateWindow(gWIDTH, gHEIGHT, "Vulkan", nullptr, nullptr);
    }

    void createInstance() {
        if (gENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }
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

        auto vExtensions = getRequiredExtensions();
        vCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vExtensions.size());
        vCreateInfo.ppEnabledExtensionNames = vExtensions.data();
        vCreateInfo.enabledLayerCount = 0;

        VkDebugUtilsMessengerCreateInfoEXT vDebugCreateInfo{};
        if (gENABLE_VALIDATION_LAYERS) {
            vCreateInfo.enabledLayerCount
                = static_cast<uint32_t>(gVALIDATION_LAYERS.size());
            vCreateInfo.ppEnabledLayerNames = gVALIDATION_LAYERS.data();
            populateDebugMessengerCreateInfo(vDebugCreateInfo);
            vCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&vDebugCreateInfo;
        } else {
            vCreateInfo.enabledLayerCount = 0;
            vCreateInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&vCreateInfo, nullptr, &mInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& rCreateInfo) { // NOLINT [readability-convert-member-functions-to-static]
        rCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        rCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT // NOLINT [hicpp-signed-bitwise]
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        rCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT // NOLINT [hicpp-signed-bitwise]
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        rCreateInfo.pfnUserCallback = debugCallback;
        rCreateInfo.pUserData = nullptr;
    }

    void setupDebugMessenger() {
        if (!gENABLE_VALIDATION_LAYERS) {
            return;
        }
        VkDebugUtilsMessengerCreateInfoEXT vCreateInfo{};
        populateDebugMessengerCreateInfo(vCreateInfo);
        if (createDebugUtilsMessengerEXT(mInstance,
                                         &vCreateInfo,
                                         nullptr,
                                         &mDebugMessenger)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    VKAPI_ATTR static auto VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT rMessageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT rMessageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* prCallbackData,
                  void* prUserData) -> VkBool32 {
        std::cerr << "validation layer: " << prCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    auto checkValidationLayerSupport() -> bool {
        uint32_t vLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&vLayerCount, nullptr);
        std::vector<VkLayerProperties> vAvailableLayers(vLayerCount);
        vkEnumerateInstanceLayerProperties(&vLayerCount, vAvailableLayers.data());

        for (const auto* pvLayerName : gVALIDATION_LAYERS) {
            bool vLayerFound = false;
            for (const auto& vLayerProperties : vAvailableLayers) {
                if (strcmp(pvLayerName, vLayerProperties.layerName) == 0) {
                    vLayerFound = true;
                    break;
                }
            }
            if (!vLayerFound) {
                return false;
            }
        }
        return true;
    }

    auto getRequiredExtensions() -> std::vector<const char*> {
        uint32_t vGlfwExtensionCount = 0;
        const char** pvGlfwExtensions
            = glfwGetRequiredInstanceExtensions(&vGlfwExtensionCount);

        std::vector<const char*> vExtensions(pvGlfwExtensions,
                                             pvGlfwExtensions + vGlfwExtensionCount);

        if (gENABLE_VALIDATION_LAYERS) {
            vExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return vExtensions;
    }

    void mainLoop() {
        while (glfwWindowShouldClose(mWindow) == 0) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        if (gENABLE_VALIDATION_LAYERS) {
            std::cerr << "Cleaning up debug utils messenger" << std::endl;
            //destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
        }
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
