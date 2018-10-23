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

const int WIDTH = 800;
const int HEIGHT = 600;
const char *WINDOW_TITLE = "Vulkan Test";

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
    }

    void createInstance()
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionsCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

        //log layers
        check_supported_layers();

        //log extensions names
        check_supported_extensions();
        //for (int i = 0; i < glfwExtensionsCount; i++) std::cout << "extension " << glfwExtensions[i] << std::endl;
        
        const std::vector<const char *> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

        createInfo.enabledExtensionCount = glfwExtensionsCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();

        if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &instance))
        {
            throw std::runtime_error("failed to create vkInstance");
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
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
//--------------tool methods----------------
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