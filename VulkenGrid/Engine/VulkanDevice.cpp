#include "VulkanDevice.h"
#include "Logger.h"
#include <stdexcept>
#include <set>
#include <vector>

VulkanDevice::VulkanDevice(VulkanInstance& instance) : instance(instance) {}

void VulkanDevice::init(VkSurfaceKHR surface) {
    Logger::getInstance().log("Initializing Vulkan Device...");
    pickPhysicalDevice(surface);
    VulkanDevice::createLogicalDevice(surface);
    VulkanDevice::createCommandPool();
    Logger::getInstance().log("Vulkan Device initialized successfully.");
}

void VulkanDevice::cleanup() {
    Logger::getInstance().log("Cleaning up Vulkan Device...");
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    Logger::getInstance().log("Vulkan Device cleaned up successfully.");
}

void VulkanDevice::pickPhysicalDevice(VkSurfaceKHR surface) {
    Logger::getInstance().log("Picking physical device...");
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        Logger::getInstance().logError("Failed to find GPUs with Vulkan support!");
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device, surface)) {
            physicalDevice = device;
            Logger::getInstance().log("Physical device selected.");
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        Logger::getInstance().logError("Failed to find a suitable GPU!");
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

void VulkanDevice::createLogicalDevice(VkSurfaceKHR surface) {
    Logger::getInstance().log("Creating logical device...");
    queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        Logger::getInstance().logError("Failed to create logical device!");
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
    Logger::getInstance().log("Logical device created successfully.");
}

void VulkanDevice::createCommandPool() {
    Logger::getInstance().log("Creating command pool...");
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        Logger::getInstance().logError("Failed to create command pool!");
        throw std::runtime_error("Failed to create command pool!");
    }
    Logger::getInstance().log("Command pool created successfully.");
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    Logger::getInstance().log("Checking if device is suitable...");
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    bool extensionsSupported = true; // Add your extension checks if necessary
    bool swapChainAdequate = false;

    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    Logger::getInstance().log("Device suitability: " + std::string(indices.isComplete() && extensionsSupported && swapChainAdequate ? "Suitable" : "Not Suitable"));
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) const {
    Logger::getInstance().log("Finding queue families...");
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            Logger::getInstance().log("Graphics queue family found at index: " + std::to_string(i));
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
            Logger::getInstance().log("Present queue family found at index: " + std::to_string(i));
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkSurfaceKHR surface) const {
    Logger::getInstance().log("Querying swap chain support...");
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
        Logger::getInstance().log("Swap chain formats found: " + std::to_string(formatCount));
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
        Logger::getInstance().log("Swap chain present modes found: " + std::to_string(presentModeCount));
    }

    return details;
}