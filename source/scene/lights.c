#include "lights.h"

void init_lights(VkDevice device,
                 VkPhysicalDevice phys)
{
	VkDescriptorSetLayoutBinding lightBinding = {0};
	lightBinding.binding = 0;
	lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightBinding.descriptorCount = 1;
	lightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &lightBinding;

	vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &lightDescriptorSetLayout);

    VkDeviceSize size = sizeof(LightingUBO);

    create_vulkan_buffer(
                  size,
                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  &g_lights.buffer,
                  &g_lights.memory);

    // Persistent map
    vkMapMemory(device, g_lights.memory, 0, size, 0, &g_lights.mapped);
}

void init_lights_write(VkDevice device)
{
    // Descriptor write (binding 0)
    VkDescriptorBufferInfo bufInfo = {0};
    bufInfo.buffer = g_lights.buffer;
    bufInfo.offset = 0;
    bufInfo.range  = sizeof(LightingUBO);

    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = g_lights.descriptorSet; 
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufInfo;

    vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
}

void lights_update(VkCommandBuffer cmdBuf,
                   VkPipelineLayout pipelineLayout,
                   float ambientR, float ambientG, float ambientB,
                   float dirX, float dirY, float dirZ,
                   float dirIntensity,
                   int numPoints,
                   float camX, float camY, float camZ)
{
    LightingUBO* ubo = (LightingUBO*)g_lights.mapped;

    // --- Ambient ---
    ubo->ambient = (Vec3Std140){ ambientR, ambientG, ambientB, 0.0f };

    // --- Directional light ---
    ubo->dirLight.direction = (Vec3Std140){ dirX, dirY, dirZ, 0.0f };
    ubo->dirLight.color     = (Vec3Std140){ 1.0f, 1.0f, 1.0f, 0.0f };
    ubo->dirLight.intensity = dirIntensity;

    ubo->numPointLights = numPoints;

    // --- Point lights ---
    ubo->pointLights[0] = (PointLightStd140){
        .position = { 5.0f, 3.0f, -2.0f, 0.0f },
        .color    = { 1.0f, 0.8f, 0.6f, 0.0f },
        .intensity = 1.5f,
        .constant = 1.0f,
        .linear = 0.09f,
        .quadratic = 0.032f
    };

    ubo->pointLights[1] = (PointLightStd140){
        .position = { -4.0f, 2.0f, 1.0f, 0.0f },
        .color    = { 0.6f, 0.8f, 1.0f, 0.0f },
        .intensity = 1.2f,
        .constant = 1.0f,
        .linear = 0.14f,
        .quadratic = 0.07f
    };

    ubo->pointLights[2] = (PointLightStd140){
        .position = { 0.0f, 4.0f, 0.0f, 0.0f },
        .color    = { 0.6f, 1.0f, 0.6f, 0.0f },
        .intensity = 2.0f,
        .constant = 1.0f,
        .linear = 0.07f,
        .quadratic = 0.017f
    };

    ubo->pointLights[3] = (PointLightStd140){
        .position = { 2.0f, 1.0f, 5.0f, 0.0f },
        .color    = { 1.0f, 0.4f, 0.4f, 0.0f },
        .intensity = 1.0f,
        .constant = 1.0f,
        .linear = 0.09f,
        .quadratic = 0.032f
    };

    // Zero unused lights
    for (int i = numPoints; i < 4; ++i) {
        memset(&ubo->pointLights[i], 0, sizeof(PointLightStd140));
    }

    // --- Camera ---
    ubo->viewPos = (Vec3Std140){ camX, camY, camZ, 0.0f };
}