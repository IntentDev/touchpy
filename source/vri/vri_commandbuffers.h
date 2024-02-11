#pragma once

#include "vri_macros.h"
#include "vri_initializers.h"
#include <memory>
#include <stdexcept>

NAMESPACE_BEGIN(vri)

class CommandBuffer 
{
public:
    CommandBuffer(VkDevice device, VkCommandPool commandPool)
        :   device_(device), 
            commandPool_(commandPool), 
            commandBuffer_(VK_NULL_HANDLE) 
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool_;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffer");
        }
    }

    ~CommandBuffer() 
    {
        if (commandBuffer_ != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer_);
        }
    }

    // Delete copy constructor and assignment operator to prevent copying
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    // Define move constructor and assignment operator for transfer of ownership
    CommandBuffer(CommandBuffer&& other) noexcept
        :   device_(other.device_), 
            commandPool_(other.commandPool_),
            commandBuffer_(other.commandBuffer_) 
    {
        other.commandBuffer_ = VK_NULL_HANDLE;
    }

    CommandBuffer& operator=(CommandBuffer&& other) noexcept 
    {
        if (this != &other) 
        {
            vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer_);
            device_ = other.device_;
            commandPool_ = other.commandPool_;
            commandBuffer_ = other.commandBuffer_;
            other.commandBuffer_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkCommandBuffer get() const { return commandBuffer_; }

private:
    VkDevice device_;
    VkCommandPool commandPool_;
    VkCommandBuffer commandBuffer_;
};


class CommandBuffers {
public:
    CommandBuffers(VkDevice device, VkCommandPool commandPool) 
        :   device_(device), 
            commandPool_(commandPool), 
            bufferCount_(1) 
    {
        // Allocate a single command buffer
        singleCommandBuffer_ = VK_NULL_HANDLE;
        VkCommandBufferAllocateInfo allocInfo = vri::commandBufferAllocateInfo(
            commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

        VK_CHECK(vkAllocateCommandBuffers(device_, &allocInfo, &singleCommandBuffer_));
    }

    CommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t bufferCount) 
        :   device_(device), 
            commandPool_(commandPool), 
            bufferCount_(bufferCount) 
    {
        if (bufferCount_ == 0)
            throw std::runtime_error("Buffer count must be at least 1");

        if (bufferCount_ > 1) 
        {
            commandBuffers_.reset(new VkCommandBuffer[bufferCount_]);
            VkCommandBufferAllocateInfo allocInfo = vri::commandBufferAllocateInfo(
                commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, bufferCount_);

            VK_CHECK(vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.get()));
        }
        else 
        {
            VkCommandBufferAllocateInfo allocInfo = vri::commandBufferAllocateInfo(
                commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

            VK_CHECK(vkAllocateCommandBuffers(device_, &allocInfo, &singleCommandBuffer_));
        }
    }

    ~CommandBuffers() 
    {
        if (bufferCount_ > 1 && commandBuffers_.get() != VK_NULL_HANDLE)
            vkFreeCommandBuffers(device_, commandPool_, bufferCount_, commandBuffers_.get());
     
        else if (bufferCount_ == 1 && singleCommandBuffer_ != VK_NULL_HANDLE)
            vkFreeCommandBuffers(device_, commandPool_, 1, &singleCommandBuffer_);
    }

    // Delete copy constructor and assignment operator to prevent copying
    CommandBuffers(const CommandBuffers&) = delete;
    CommandBuffers& operator=(const CommandBuffers&) = delete;

    // Move constructor
    CommandBuffers(CommandBuffers&& other) noexcept
        :   device_(other.device_),
            commandPool_(other.commandPool_),
            bufferCount_(other.bufferCount_),
            commandBuffers_(std::move(other.commandBuffers_)),
            singleCommandBuffer_(other.singleCommandBuffer_)
    {
        other.singleCommandBuffer_ = VK_NULL_HANDLE;
        other.bufferCount_ = 0;  // Reset the buffer count of the moved-from object
    }

    // Move assignment operator
    CommandBuffers& operator=(CommandBuffers&& other) noexcept
    {
        if (this != &other)
        {
            // Free existing resources
            if (bufferCount_ > 1) {
                vkFreeCommandBuffers(device_, commandPool_, bufferCount_, commandBuffers_.get());
            }
            else if (bufferCount_ == 1) {
                vkFreeCommandBuffers(device_, commandPool_, 1, &singleCommandBuffer_);
            }

            // Transfer ownership
            device_ = other.device_;
            commandPool_ = other.commandPool_;
            bufferCount_ = other.bufferCount_;
            commandBuffers_ = std::move(other.commandBuffers_);
            singleCommandBuffer_ = other.singleCommandBuffer_;

            // Reset the moved-from object
            other.singleCommandBuffer_ = VK_NULL_HANDLE;
            other.bufferCount_ = 0;
        }
        return *this;
    }


    VkCommandBuffer get(size_t index = 0) const {
        if (bufferCount_ == 1) {
            if (index != 0) throw std::runtime_error("Invalid index for a single command buffer");
            return singleCommandBuffer_;
        }
        else {
            if (index >= bufferCount_) throw std::runtime_error("Index out of bounds");
            return commandBuffers_[index];
        }
    }

private:
    VkDevice device_;
    VkCommandPool commandPool_;
    uint32_t bufferCount_;
    std::unique_ptr<VkCommandBuffer[]> commandBuffers_; 
    VkCommandBuffer singleCommandBuffer_{ VK_NULL_HANDLE }; 
};

NAMESPACE_END(vri)