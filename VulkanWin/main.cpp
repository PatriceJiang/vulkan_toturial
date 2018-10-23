#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <map>

const int WIDTH = 800;
const int HEIGHT = 600;
const char *WINDOW_TITLE = "Vulkan Test";

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char *layerPrefix,
    const char *msg,
    void *userData
)
{
    std::cerr << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    bool isComplete()
    {
        return graphicsFamily >= 0;
    }
};

class HelloTriangleApplication
{
public:
    void run() 
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
private:
    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);

    }
    void initVulkan()
    {
        createInstance();
        setupDebugCallback();
        pickupPhysicalDevice();
        createLogicalDevice();
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures features = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &features;
    }

    void pickupPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        
        //std::map<int, VkPhysicalDevice> candidates;
        //int max = 0;
        //for (const auto &device : devices)
        //{
        //    int score = rateDeviceSuitability(device);
        //    if (score > max) {
        //        max = score;
        //    }
        //    candidates.insert(std::make_pair(score, device));
        //}
        //physicalDevice = candidates[max];

        for (const auto &device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void createInstance()
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        //appInfo.apiVersion = VK_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();

        //log layers
        check_supported_layers();

        //log extensions names
        check_supported_extensions();
        //for (int i = 0; i < glfwExtensionsCount; i++) std::cout << "extension " << glfwExtensions[i] << std::endl;
        
        const std::vector<const char *> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();

        if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &instance))
        {
            throw std::runtime_error("failed to create vkInstance");
        }

    }

    void setupDebugCallback()
    {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;
        createInfo.pUserData = this;

        if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to setup debug callback!");
        }
    }

    VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
        auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }
    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func != nullptr) {
            func(instance, callback, pAllocator);
        }
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }
    void cleanup()
    {
        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
//--------------tool methods----------------



    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

        int i = 0;
        for (const auto &family : properties)
        {
            if (family.queueCount > 0 && family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            if (indices.isComplete())
            {
                break;
            }
            i++;
        }

        return indices;
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        auto indices = findQueueFamilies(device);
        return indices.isComplete();
    }

    int rateDeviceSuitability(VkPhysicalDevice device)
    {
        int score = 0;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 10000;
        }
        if (features.geometryShader) {
            score += 1000;
        }
        return score;
    }

    void check_supported_extensions()
    {
        uint32_t extensionCount; 
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> properties(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, properties.data());
        for (auto prop : properties)
        {
            std::cout << "support property : " << prop.extensionName << " version " << prop.specVersion << std::endl;
        }
    }

    std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionsCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        return extensions;
    }

    std::vector<VkLayerProperties> check_supported_layers(bool log = true)
    {
        uint32_t layers;
        vkEnumerateInstanceLayerProperties(&layers, nullptr);
        std::vector<VkLayerProperties> layerVector(layers);
        vkEnumerateInstanceLayerProperties(&layers, layerVector.data());
        if(log)
        {
            for (auto l : layerVector)
            {
                std::cout << "supported layer: " << l.layerName << std::endl;
            }
        }
        return layerVector;
    }

    void has_layer(const char *layername)
    {
        auto layers = check_supported_layers(false);
        bool found = false;
        for (auto l : layers)
        {
            if (strcmp(l.layerName, layername) == 0)
            {
                found = true;
                break;
            }
        }

    }

//----------------- fields ---------------- 
private:
    GLFWwindow *window = nullptr;
    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};


int main()
{
    HelloTriangleApplication app;
    app.run();
    return EXIT_SUCCESS;
}

int first_main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported" << std::endl;

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for (auto prop : extensions)
    {
        std::cout << "extension " << prop.extensionName << std::endl;
    }

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}