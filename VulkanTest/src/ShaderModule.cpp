#include <VulkanTest/ShaderModule.h>
#include <VulkanTest/Renderer.h>

#include <fstream>
#include <iostream>

VulkanTest::ShaderModule::ShaderModule( const std::string& file_path, vk::ShaderStageFlagBits shader_stage_flag ) 
  : vk_shader_stage_flag( shader_stage_flag ) {

  std::vector< char > bytecode;
  readSource( file_path, bytecode );

  auto shader_module_info = vk::ShaderModuleCreateInfo()
    .setPCode( reinterpret_cast< const uint32_t* >( bytecode.data() ) )
    .setCodeSize( bytecode.size() );

  vk_shader_module = Renderer::get()->getVkDevice().createShaderModule( shader_module_info );

}

VulkanTest::ShaderModule::~ShaderModule() {
  Renderer::get()->getVkDevice().destroyShaderModule( vk_shader_module );
}

const vk::ShaderStageFlagBits VulkanTest::ShaderModule::getVkShaderStageFlag() {
  return vk_shader_stage_flag;
}

const vk::ShaderModule& VulkanTest::ShaderModule::getVkShaderModule() {
  return vk_shader_module;
}

void VulkanTest::ShaderModule::readSource( const std::string& file_path, std::vector< char >& bytecode ) {

  std::ifstream file( file_path, std::ios::ate | std::ios::binary );

  if( !file.is_open() ) {
    std::cerr << "Could not open shader file " << file_path << std::endl;
    return;
  }

  const size_t file_size = static_cast< size_t >( file.tellg() );
  bytecode.resize( file_size );
  file.seekg( 0 );
  file.read( bytecode.data(), file_size );
  file.close();

}