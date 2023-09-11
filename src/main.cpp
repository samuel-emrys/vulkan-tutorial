#include <cstdint>
#include <cstring>
#include <vulkan/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

#include "queuefamilies.h"

const uint32_t gWIDTH = 800;
const uint32_t gHEIGHT = 800;
const std::vector<const char*> gVALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool gENABLE_VALIDATION_LAYERS = false;
#else
const bool gENABLE_VALIDATION_LAYERS = true;
#endif

auto createDebugUtilsMessengerEXT(
    VkInstance prInstance,
    const VkDebugUtilsMessengerCreateInfoEXT* prCreateInfo,
    const VkAllocationCallbacks* prAllocator,
    VkDebugUtilsMessengerEXT* prDebugMessenger
) -> VkResult {
    auto pvFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(prInstance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (pvFunc != nullptr) {
        return pvFunc(prInstance, prCreateInfo, prAllocator, prDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

auto destroyDebugUtilsMessengerEXT(
    VkInstance prInstance,
    VkDebugUtilsMessengerEXT prDebugMessenger,
    const VkAllocationCallbacks* prAllocator
) {
    auto pvFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(prInstance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (pvFunc != nullptr) {
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
    VkSurfaceKHR mSurface;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mLogicalDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

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
            vCreateInfo.pNext = &vDebugCreateInfo;
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
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
    }

    void createSurface() {
        if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t vDeviceCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &vDeviceCount, nullptr);

        if (vDeviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> vDevices(vDeviceCount);
        vkEnumeratePhysicalDevices(mInstance, &vDeviceCount, vDevices.data());

        for (const auto& vDevice : vDevices) {
            if (isDeviceSuitable(vDevice)) {
                mPhysicalDevice = vDevice;
                break;
            }
        }

        if (mPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    auto isDeviceSuitable(VkPhysicalDevice prDevice) -> bool {
        VkPhysicalDeviceProperties vDeviceProperties;
        VkPhysicalDeviceFeatures vDeviceFeatures;
        vkGetPhysicalDeviceProperties(prDevice, &vDeviceProperties);
        vkGetPhysicalDeviceFeatures(prDevice, &vDeviceFeatures);

        QueueFamilyIndices vIndices = findQueueFamilies(prDevice);
        return vIndices.isComplete();
    }

    auto findQueueFamilies(VkPhysicalDevice prDevice) -> QueueFamilyIndices {
        QueueFamilyIndices vIndices{};

        uint32_t vQueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(prDevice, &vQueueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> vQueueFamilies(vQueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            prDevice,
            &vQueueFamilyCount,
            vQueueFamilies.data()
        );

        size_t vIndex{};
        auto vPresentSupport = static_cast<VkBool32>(false);
        for (const auto& vQueueFamily : vQueueFamilies) {
            if (static_cast<bool>(vQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                vIndices.graphicsFamily = vIndex;
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    prDevice,
                    vIndex,
                    mSurface,
                    &vPresentSupport
                );
                if (static_cast<bool>(vPresentSupport)) {
                    vIndices.presentFamily = vIndex;
                }
            }
            if (vIndices.isComplete()) {
                break;
            }
            vIndex++;
        }

        return vIndices;
    }

    void createLogicalDevice() {
        QueueFamilyIndices vIndices = findQueueFamilies(mPhysicalDevice);
        std::vector<VkDeviceQueueCreateInfo> vQueueCreateInfos{};
        std::set<uint32_t> vUniqueQueueFamilies
            = {vIndices.graphicsFamily.value(), vIndices.presentFamily.value()};

        float vQueuePriority = 1.0F;
        for (uint32_t vQueueFamily : vUniqueQueueFamilies) {
            VkDeviceQueueCreateInfo vQueueCreateInfo{};
            vQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            vQueueCreateInfo.queueFamilyIndex = vQueueFamily;
            vQueueCreateInfo.queueCount = 1;
            vQueueCreateInfo.pQueuePriorities = &vQueuePriority;
            vQueueCreateInfos.push_back(vQueueCreateInfo);
        }

        VkPhysicalDeviceFeatures vDeviceFeatures{};

        VkDeviceCreateInfo vCreateInfo{};
        vCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        vCreateInfo.queueCreateInfoCount
            = static_cast<uint32_t>(vQueueCreateInfos.size());
        vCreateInfo.pQueueCreateInfos = vQueueCreateInfos.data();
        vCreateInfo.pEnabledFeatures = &vDeviceFeatures;
        vCreateInfo.enabledExtensionCount = 0;

        if (gENABLE_VALIDATION_LAYERS) {
            vCreateInfo.enabledLayerCount
                = static_cast<uint32_t>(gVALIDATION_LAYERS.size());
            vCreateInfo.ppEnabledLayerNames = gVALIDATION_LAYERS.data();
        } else {
            vCreateInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(mPhysicalDevice, &vCreateInfo, nullptr, &mLogicalDevice)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        vkGetDeviceQueue(
            mLogicalDevice,
            vIndices.graphicsFamily.value(),
            0,
            &mGraphicsQueue
        );
        vkGetDeviceQueue(
            mLogicalDevice,
            vIndices.presentFamily.value(),
            0,
            &mPresentQueue
        );
    }

    // [readability-convert-member-functions-to-static]
    void populateDebugMessengerCreateInfo( // NOLINT
        VkDebugUtilsMessengerCreateInfoEXT& rCreateInfo
    ) {
        rCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        rCreateInfo.messageSeverity
            = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT // NOLINT [hicpp-signed-bitwise]
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        rCreateInfo.messageType
            = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT // NOLINT [hicpp-signed-bitwise]
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
        if (createDebugUtilsMessengerEXT(
                mInstance,
                &vCreateInfo,
                nullptr,
                &mDebugMessenger
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    VKAPI_ATTR static auto VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT rMessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT rMessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* prCallbackData,
        void* prUserData
    ) -> VkBool32 {
        std::cerr << "validation layer: " << prCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    // [readability-convert-member-functions-to-static]
    auto checkValidationLayerSupport() -> bool { // NOLINT
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

    // [readability-convert-member-functions-to-static]
    auto getRequiredExtensions() -> std::vector<const char*> { // NOLINT
        uint32_t vGlfwExtensionCount = 0;
        const char** pvGlfwExtensions
            = glfwGetRequiredInstanceExtensions(&vGlfwExtensionCount);

        std::vector<const char*> vExtensions(
            pvGlfwExtensions,
            pvGlfwExtensions + vGlfwExtensionCount
        );

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
            destroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
        }
        vkDestroyDevice(mLogicalDevice, nullptr);
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
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
