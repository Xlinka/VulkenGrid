#pragma once

#include "VulkanInstance.h"
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>
#include <string>
#include <set>
#include "Logger.h"

#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanDevice {
public:
    VulkanDevice(VulkanInstance& instance);
    void init(VkSurfaceKHR surface);
    void cleanup();
    VkDevice getDevice() const { return device; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    VkCommandPool getCommandPool() const { return commandPool; }
    QueueFamilyIndices getQueueFamilyIndices() const { return queueFamilyIndices; }

    // Overloaded function
    SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface) const;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) const;

private:
    VulkanInstance& instance;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    QueueFamilyIndices queueFamilyIndices;

    void pickPhysicalDevice(VkSurfaceKHR surface);
    void createLogicalDevice(VkSurfaceKHR surface);
    void createCommandPool();
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    std::vector<const char*> getDeviceExtensions() const;
};

#endif // VULKAN_DEVICE_H
