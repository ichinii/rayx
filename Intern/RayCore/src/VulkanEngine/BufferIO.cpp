#include "VulkanEngine/VulkanEngine.h"

namespace RAYX {
void VulkanEngine::storeToStagingBuffer(std::vector<char> data) {
    void* buf;
    vkMapMemory(m_Device, m_stagingMemory, 0, STAGING_SIZE, 0, &buf);

    memcpy(buf, data.data(), data.size());

    vkUnmapMemory(m_Device, m_stagingMemory);
}

std::vector<char> VulkanEngine::loadFromStagingBuffer(uint32_t bytes) {
    std::vector<char> data(bytes);
    void* buf;

    vkMapMemory(m_Device, m_stagingMemory, 0, bytes, 0, &buf);

    memcpy(data.data(), buf, bytes);

    vkUnmapMemory(m_Device, m_stagingMemory);

    return data;
}

void VulkanEngine::gpuMemcpy(VkBuffer& buffer_src, uint32_t offset_src,
                             VkBuffer& buffer_dst, uint32_t offset_dst,
                             uint32_t bytes) {
    RAYX_PROFILE_FUNCTION();
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = offset_src;
    copyRegion.dstOffset = offset_dst;
    copyRegion.size = bytes;

    vkCmdCopyBuffer(commandBuffer, buffer_src, buffer_dst, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_ComputeQueue);

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

}  // namespace RAYX