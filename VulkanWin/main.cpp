#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>
#include <set>

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <map>

#include <fstream>

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

template<typename T>
class optional {
    T _v;
    bool _hasValue = false;
public:
    T value() { assert(_hasValue); return _v; }
    bool has_value() { return _hasValue; }
    optional(const T &v) :_hasValue(true), _v(v) {}
    optional() {}
};

struct QueueFamilyIndices {
    optional<uint32_t> graphicsFamily = -1;
    optional<uint32_t> presentFamily = -1;
   
    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char *> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


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
        createSurface();
        setupDebugCallback();
        pickupPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createGraphicsPipeline();
    }

    void createGraphicsPipeline()
    {
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

    }

    void createShaderModule(const std::vector<char> &code)
    {

    }

    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());
        for (auto i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        auto extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        if (indices.graphicsFamily.value() != indices.presentFamily.value())
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; //Optional
            createInfo.pQueueFamilyIndices = nullptr; //Optional
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (VK_SUCCESS != vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain))
        {
            throw std::runtime_error("failed to create swapchain!");
        }

        imageCount = 0;
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw new std::runtime_error("failed to create surface!");
        }
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;

        for (auto queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures features = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &features;
        createInfo.enabledExtensionCount = 0;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();


        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        if (VK_SUCCESS != vkCreateDevice(physicalDevice, &createInfo, nullptr, &device))
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

 
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
        for (auto v : swapChainImageViews)
        {
            vkDestroyImageView(device, v, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);

        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
//--------------tool methods----------------

    static std::vector<char> readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file " + filename);
        }

        size_t fsize = (size_t)file.tellg();
        std::vector<char> buffer(fsize);
        file.seekg(0);
        file.read(buffer.data(), fsize);
        file.close();
        return buffer;
    }


    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaiblePresentModes)
    {
        VkPresentModeKHR best = VK_PRESENT_MODE_FIFO_KHR;
        for(const auto &mode: avaiblePresentModes)
        { 
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return mode;
            }
            else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                best = mode;
            }

        }
        return best;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtend = { WIDTH, HEIGHT };
            actualExtend.width = std::max(capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtend.width));
            actualExtend.height = std::max(capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtend.height));

            return actualExtend;
        }
    }


    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        uint32_t formatCount; 
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }


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
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (family.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = i;
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
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount; 
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
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
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
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