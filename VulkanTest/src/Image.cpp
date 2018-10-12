#ifndef IMAGE_CPP
#define IMAGE_CPP

#include <VulkanTest/Image.h>
#include <VulkanTest/VulkanManager.h>

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
VulkanTest::Image< format, image_type, tiling, sample_count_flags >::Image( 
  vk::ImageLayout initial_layout,
  vk::ImageUsageFlags usage_flags,
  VmaMemoryUsage vma_memory_usage,
  uint32_t _width,
  uint32_t _height,
  uint32_t _depth,
  size_t pixel_size ) 
  : width( _width ), height( _height ), depth( _depth ), vk_image_layout( initial_layout ), data_size( pixel_size * width * height * depth ) {
  createImage( usage_flags, vma_memory_usage );
}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
VulkanTest::Image< format, image_type, tiling, sample_count_flags >::~Image() {
  VulkanManager::getInstance()->getVkDevice().destroyImageView( vk_image_view );
  vmaDestroyImage( VulkanManager::getInstance()->getVmaAllocator(), vk_image, vma_allocation );
}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
void VulkanTest::Image< format, image_type, tiling, sample_count_flags >::setImageData( const void* data ) {
  updateBuffer( data, data_size );
}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
void VulkanTest::Image< format, image_type, tiling, sample_count_flags >::createImageView( vk::ImageViewType image_view_type, vk::ImageAspectFlags image_aspect_flags ) {

  auto subresource_range = vk::ImageSubresourceRange()
    .setAspectMask( image_aspect_flags )
    .setBaseMipLevel( 0 )
    .setLevelCount( 1 )
    .setBaseArrayLayer( 0 )
    .setLayerCount( 1 );

  auto image_view_create_info = vk::ImageViewCreateInfo()
    .setFormat( format )
    .setImage( vk_image )
    .setViewType( image_view_type )
    .setSubresourceRange( subresource_range );

  vk_image_view = VulkanManager::getInstance()->getVkDevice().createImageView( image_view_create_info );
  if( !vk_image_view ){
    throw std::runtime_error( "Could not create image view for image!" );
  }

}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
void VulkanTest::Image< format, image_type, tiling, sample_count_flags >::transitionImageLayout( 
  vk::ImageLayout new_layout, const vk::CommandBuffer& command_buffer ) {

  const vk::CommandBuffer& command_buffer_to_use = command_buffer ? command_buffer : single_use_command_buffer;
  bool created_single_use_command_buffer = false;
  if( !command_buffer_to_use ) {
    created_single_use_command_buffer = true;
    beginSingleUsageCommandBuffer();
  }

  auto subresource_range = vk::ImageSubresourceRange()
    .setAspectMask( vk::ImageAspectFlagBits::eColor )
    .setBaseMipLevel( 0 )
    .setLevelCount( 1 )
    .setBaseArrayLayer( 0 )
    .setLayerCount( 1 );

  vk::PipelineStageFlagBits source_stage;
  vk::PipelineStageFlagBits destination_stage;

  auto image_memory_barrier = vk::ImageMemoryBarrier()
    .setImage( vk_image )
    .setOldLayout( vk_image_layout )
    .setNewLayout( new_layout )
    .setSrcQueueFamilyIndex( VK_QUEUE_FAMILY_IGNORED )
    .setDstQueueFamilyIndex( VK_QUEUE_FAMILY_IGNORED );

  if( vk_image_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal ) {

    image_memory_barrier.setSrcAccessMask( vk::AccessFlags() );
    image_memory_barrier.setDstAccessMask( vk::AccessFlagBits::eTransferWrite );

    source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    destination_stage = vk::PipelineStageFlagBits::eTransfer;

  } else if( vk_image_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal ) {

    subresource_range.setAspectMask( vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil );

    image_memory_barrier.setSrcAccessMask( vk::AccessFlags() );
    image_memory_barrier.setDstAccessMask( vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite );

    source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
    destination_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;

  } else if( vk_image_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal ) {
  
    image_memory_barrier.setSrcAccessMask( vk::AccessFlagBits::eTransferWrite );
    image_memory_barrier.setDstAccessMask( vk::AccessFlagBits::eShaderRead );

    source_stage = vk::PipelineStageFlagBits::eTransfer;
    destination_stage = vk::PipelineStageFlagBits::eFragmentShader;

  } else {
    throw std::runtime_error( "Invalid image transfer request!" );
  }

  image_memory_barrier.setSubresourceRange( subresource_range );
  command_buffer_to_use.pipelineBarrier( source_stage, destination_stage, vk::DependencyFlags(), 0, 0, image_memory_barrier );

  vk_image_layout = new_layout;

  if( created_single_use_command_buffer ){
    endSingleUsageCommandBuffer();
  }

}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
const vk::Format VulkanTest::Image< format, image_type, tiling, sample_count_flags >::getVkFormat() {
  return format;
}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
const vk::SampleCountFlagBits VulkanTest::Image< format, image_type, tiling, sample_count_flags >::getVkSampleCountFlags() {
  return sample_count_flags;
}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
const vk::ImageView& VulkanTest::Image< format, image_type, tiling, sample_count_flags >::getVkImageView() {
  return vk_image_view;
}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
void VulkanTest::Image< format, image_type, tiling, sample_count_flags >::insertTransferCommand( 
  const vk::CommandBuffer& command_buffer, const vk::Buffer& source_buffer ) {

  transitionImageLayout( vk::ImageLayout::eTransferDstOptimal, command_buffer );

  auto image_subresource = vk::ImageSubresourceLayers()
    .setAspectMask( vk::ImageAspectFlagBits::eColor )
    .setMipLevel( 0 )
    .setBaseArrayLayer( 0 )
    .setLayerCount( 1 );

  auto image_offset = vk::Offset3D()
    .setX( 0 )
    .setY( 0 )
    .setZ( 0 );

  auto image_extent = vk::Extent3D()
    .setWidth( width )
    .setHeight( height )
    .setDepth( depth );

  auto buffer_image_copy = vk::BufferImageCopy()
    .setBufferOffset( 0 )
    .setBufferRowLength( 0 )
    .setBufferImageHeight( 0 )
    .setImageSubresource( image_subresource )
    .setImageOffset( image_offset )
    .setImageExtent( image_extent );

  command_buffer.copyBufferToImage( source_buffer, vk_image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copy );

  transitionImageLayout( vk::ImageLayout::eShaderReadOnlyOptimal, command_buffer );

}

/// Override to return the required data size of the staging buffer in order to transfer all data to this Image
/// \return The data size for the staging buffer
template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
size_t VulkanTest::Image< format, image_type, tiling, sample_count_flags >::getStagingBufferSize() {
  return data_size;
}

template< vk::Format format, vk::ImageType image_type, vk::ImageTiling tiling, vk::SampleCountFlagBits sample_count_flags >
void VulkanTest::Image< format, image_type, tiling, sample_count_flags >::createImage( 
  vk::ImageUsageFlags usage_flags,
  VmaMemoryUsage vma_usage ) {

  auto image_extent = vk::Extent3D()
    .setWidth( width )
    .setHeight( height )
    .setDepth( depth );

  auto image_info = vk::ImageCreateInfo()
    .setImageType( image_type )
    .setExtent( image_extent )
    .setMipLevels( 1 )
    .setArrayLayers( 1 )
    .setFormat( format )
    .setTiling( tiling )
    .setInitialLayout( vk_image_layout )
    .setUsage( usage_flags )
    .setSharingMode( vk::SharingMode::eExclusive )
    .setSamples( sample_count_flags );

  vk_image = VulkanManager::getInstance()->getVkDevice().createImage( image_info );
  if( !vk_image ) {
    throw std::runtime_error( "Could not create image!" );
  }

  VmaAllocationCreateInfo allocate_info = {};
  allocate_info.usage = vma_usage;

  VkImage c_image_handle;
  auto allocation_result = vmaCreateImage( 
    VulkanManager::getInstance()->getVmaAllocator(),
    &static_cast< VkImageCreateInfo >( image_info ),
    &allocate_info,
    &c_image_handle,
    &vma_allocation,
    nullptr );
  if( allocation_result != VK_SUCCESS ) {
    throw std::runtime_error( "Could not allocate Image memory!" );
  }

  vk_image = c_image_handle;

}

#endif /* IMAGE_CPP */