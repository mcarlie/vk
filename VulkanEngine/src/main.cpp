#define TINYOBJLOADER_IMPLEMENTATION

#include <VulkanEngine/GLFWWindow.h>
#include <VulkanEngine/UniformBuffer.h>
#include <VulkanEngine/ShaderImage.h>
#include <VulkanEngine/StagedBuffer.h>
#include <VulkanEngine/OBJLoader.h>

#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct MvpUbo {
  Eigen::Matrix4f model;
  Eigen::Matrix4f view;
  Eigen::Matrix4f projection;
};

using RGBATexture2D1S 
  = VulkanEngine::StagedBuffer< 
  VulkanEngine::ShaderImage< 
  vk::Format::eR8G8B8A8Unorm,
  vk::ImageType::e2D,
  vk::ImageTiling::eOptimal,
  vk::SampleCountFlagBits::e1 > >;

int main() {

  std::shared_ptr< VulkanEngine::Window > window( new VulkanEngine::GLFWWindow( 1280, 800, "VulkanEngine", false ) );
  window->initialize();

  auto vulkan_manager = VulkanEngine::VulkanManager::getInstance();
  vulkan_manager->initialize( window );

  std::shared_ptr< VulkanEngine::MeshBase > mesh;
  mesh = VulkanEngine::OBJLoader::loadOBJ( "C:/Users/Michael/Desktop/VK/VulkanEngine/models/spider_pumpkin_obj.obj", "" )[0];

  std::shared_ptr< VulkanEngine::ShaderModule > fragment_shader( 
    new VulkanEngine::ShaderModule( "C:/Users/Michael/Desktop/VK/VulkanEngine/shaders/frag.spv", vk::ShaderStageFlagBits::eFragment ) );
  std::shared_ptr< VulkanEngine::ShaderModule > vertex_shader( 
    new VulkanEngine::ShaderModule( "C:/Users/Michael/Desktop/VK/VulkanEngine/shaders/vert.spv", vk::ShaderStageFlagBits::eVertex ) );
  std::shared_ptr< VulkanEngine::Shader > shader( new VulkanEngine::Shader( { fragment_shader, vertex_shader } ) );

  mesh->setShader( shader );

  int texture_width;
  int texture_height;
  int channels_in_file;
  unsigned char* image_data = stbi_load( 
    "C:/Users/Michael/Desktop/VK/VulkanEngine/models/spider_pumpkin_obj_0.jpg",
    &texture_width, &texture_height,
    &channels_in_file, 4 );

  std::shared_ptr< RGBATexture2D1S > texture;
  texture.reset( new RGBATexture2D1S( 
    vk::ImageLayout::eUndefined,
    vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled, /// TODO These could be template parameters instead
    VMA_MEMORY_USAGE_GPU_ONLY,
    static_cast< uint32_t >( texture_width ),
    static_cast< uint32_t >( texture_height ),
    1, sizeof( unsigned char ) * 4, 1,
    1, // TODO
    vk::DescriptorType::eCombinedImageSampler,
    vk::ShaderStageFlagBits::eFragment ) );

  texture->setImageData( image_data );
  texture->createImageView( vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor );
  texture->createSampler();

  std::vector< std::shared_ptr< VulkanEngine::UniformBuffer< MvpUbo > > > uniform_buffers;
  uniform_buffers.resize( 3 );
  for( auto& ub : uniform_buffers ) {
    ub.reset( new VulkanEngine::UniformBuffer< MvpUbo >( 0 ) );
  }

  std::vector< std::vector< std::shared_ptr< VulkanEngine::Descriptor > > > descriptors;
  for( size_t i = 0; i < 3; ++i ) {
    descriptors.push_back( { texture, uniform_buffers[i] } );
  }

  shader->setDescriptors( descriptors );

  std::shared_ptr< VulkanEngine::Camera< float > > camera;
  camera.reset( new VulkanEngine::Camera< float >( 
    { 0, 150, -50 },
    { 0, 0, 0 },
    { 0, 1, 0 },
    0.1,
    1000,
    45,
    window->getWidth(),
    window->getHeight() ) );

  vulkan_manager->createGraphicsPipeline( mesh );
  vulkan_manager->createCommandBuffers( mesh );

  mesh->transferBuffers();
  texture->transferBuffer();

  while( !window->shouldClose() ) {

    MvpUbo vp_data;
    vp_data.projection = camera->getPerspectiveProjectionMatrix();
    vp_data.view = camera->getViewMatrix();
    for( auto& ub : uniform_buffers ) {
      ub->updateBuffer( &vp_data, sizeof( vp_data ) );
    }
    vulkan_manager->drawImage();
    window->update();

  }

  return 0;

}