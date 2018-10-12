#include <VulkanTest/SingleUsageCommandBuffer.h>
#include <VulkanTest/VulkanManager.h>

void VulkanTest::SingleUsageCommandBuffer::beginSingleUsageCommandBuffer() {

  auto command_buffer_info = vk::CommandBufferAllocateInfo()
    .setCommandBufferCount( 1 )
    .setCommandPool( VulkanManager::getInstance()->getVkCommandPool() )
    .setLevel( vk::CommandBufferLevel::ePrimary );

  auto command_buffers = VulkanManager::getInstance()->getVkDevice().allocateCommandBuffers( command_buffer_info );
  if( command_buffers.empty() ){
    throw std::runtime_error( "Could not allocate command buffer for buffer transfer!" );
  }

  single_use_command_buffer = command_buffers[0];

  auto command_buffer_begin_info = vk::CommandBufferBeginInfo()
    .setFlags( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

  single_use_command_buffer.begin( command_buffer_begin_info );

}

void VulkanTest::SingleUsageCommandBuffer::endSingleUsageCommandBuffer() {

  single_use_command_buffer.end();

  auto submit_info = vk::SubmitInfo()
    .setCommandBufferCount( 1 )
    .setPCommandBuffers( &single_use_command_buffer );

  VulkanManager::getInstance()->getVkGraphicsQueue().submit( submit_info, nullptr );
  VulkanManager::getInstance()->getVkGraphicsQueue().waitIdle(); // TODO

  VulkanManager::getInstance()->getVkDevice().freeCommandBuffers( VulkanManager::getInstance()->getVkCommandPool(), single_use_command_buffer );

}