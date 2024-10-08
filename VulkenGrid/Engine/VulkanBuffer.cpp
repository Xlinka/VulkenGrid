#include "VulkanBuffer.h"
#include <stdexcept>

void VulkanBuffer::logMemoryInfo(const char* action, VkDeviceSize size) {
    std::cout << action << ": " << size / (1024 * 1024) << " MB allocated" << std::endl; // Log in MB
}

void VulkanBuffer::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, 
                                 VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                                 VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // Create the Vulkan buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    // Log the required memory size
    logMemoryInfo("Memory requirements", memRequirements.size);

    // Allocate memory for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    // Log the allocated memory size
    logMemoryInfo("Allocated buffer memory", allocInfo.allocationSize);

    // Bind the buffer with the allocated memory
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanBuffer::cleanup(VkDevice device, VkBuffer buffer, VkDeviceMemory bufferMemory) {
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, bufferMemory, nullptr);

    // Log the cleanup action
    std::cout << "Cleaned up buffer and memory" << std::endl;
}

uint32_t VulkanBuffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
