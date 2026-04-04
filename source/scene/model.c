#define CGLTF_IMPLEMENTATION
#define CGLTF_MESHOPT_DECODE 1
#define MESHOPTIMIZER_IMPLEMENTATION
#include "cgltf.h"
#include "scene/model.h"
#include "rendering/renderer.h" 
#include "rendering/shaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
extern VkDevice device;                     // needed because renderer.h may not expose it directly
extern VkPipeline modelPipeline;
extern VkPipelineLayout modelPipelineLayout;
extern VkDescriptorSet modelDescriptorSet;
extern void create_vulkan_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkBuffer* buffer, VkDeviceMemory* memory);
extern VkRenderPass renderPass;
extern VkShaderModule modelVertModule;
extern VkShaderModule modelFragModule;
extern VkDescriptorSetLayout modelDescriptorSetLayout;

VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorPool modelDescriptorPool = VK_NULL_HANDLE;
VkDescriptorSet modelDescriptorSet = VK_NULL_HANDLE;
VkPipeline modelPipeline = VK_NULL_HANDLE;
VkPipelineLayout modelPipelineLayout = VK_NULL_HANDLE;
VkShaderModule modelVertModule = VK_NULL_HANDLE;
VkShaderModule modelFragModule = VK_NULL_HANDLE;

void create_model_pipeline(void)
{
    size_t vert_size, frag_size;
    uint32_t* vert_code = load_spirv("../../shaders/model.vert.spv", &vert_size);
    uint32_t* frag_code = load_spirv("../../shaders/model.frag.spv", &frag_size);
    if (!vert_code || !frag_code) {
        printf("Failed to load model shaders!\n");
        exit(1);
    }

    VkShaderModuleCreateInfo sm = {0};
    sm.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    sm.codeSize = vert_size; sm.pCode = vert_code;
    vkCreateShaderModule(device, &sm, NULL, &modelVertModule);
    free(vert_code);

    sm.codeSize = frag_size; sm.pCode = frag_code;
    vkCreateShaderModule(device, &sm, NULL, &modelFragModule);
    free(frag_code);

    VkPipelineShaderStageCreateInfo stages[2] = {0};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = modelVertModule;
    stages[0].pName = "main";

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = modelFragModule;
    stages[1].pName = "main";

    // Vertex input
    VkVertexInputBindingDescription binding = {0};
    binding.binding = 0;
    binding.stride = sizeof(Vertex3D);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[3] = {0};
    attrs[0].location = 0; attrs[0].binding = 0; attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[0].offset = offsetof(Vertex3D, x);
    attrs[1].location = 1; attrs[1].binding = 0; attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT; attrs[1].offset = offsetof(Vertex3D, nx);
    attrs[2].location = 2; attrs[2].binding = 0; attrs[2].format = VK_FORMAT_R32G32_SFLOAT;    attrs[2].offset = offsetof(Vertex3D, u);

    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;
    vertexInput.vertexAttributeDescriptionCount = 3;
    vertexInput.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster = {0};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.lineWidth = 1.0f;
    raster.cullMode = VK_CULL_MODE_BACK_BIT;
    raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo ms = {0};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState blendAttach = {0};
    blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blend = {0};
    blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend.attachmentCount = 1;
    blend.pAttachments = &blendAttach;

    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn = {0};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    // === NEW: descriptor layout instead of push constant ===
    VkPipelineLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &modelDescriptorSetLayout;   // <-- this is the key change

    if (vkCreatePipelineLayout(device, &layoutInfo, NULL, &modelPipelineLayout) != VK_SUCCESS) {
        printf("Failed to create model pipeline layout\n");
        exit(1);
    }

    VkGraphicsPipelineCreateInfo pipeInfo = {0};
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.stageCount = 2;
    pipeInfo.pStages = stages;
    pipeInfo.pVertexInputState = &vertexInput;
    pipeInfo.pInputAssemblyState = &inputAssembly;
    pipeInfo.pViewportState = &viewportState;
    pipeInfo.pRasterizationState = &raster;
    pipeInfo.pMultisampleState = &ms;
    pipeInfo.pDepthStencilState = &depthStencil;
    pipeInfo.pColorBlendState = &blend;
    pipeInfo.pDynamicState = &dyn;
    pipeInfo.layout = modelPipelineLayout;
    pipeInfo.renderPass = renderPass;
    pipeInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeInfo, NULL, &modelPipeline) != VK_SUCCESS) {
        printf("Failed to create model pipeline\n");
        exit(1);
    }

    printf("Model pipeline created successfully\n");
}

// Create / resize the instance storage buffer
static void create_instance_buffer(uint32_t capacity)
{
    VkDeviceSize newSize = (VkDeviceSize)capacity * sizeof(InstanceData);

    // Destroy old buffer if it exists
    if (g_model_system.instanceBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, g_model_system.instanceBuffer, NULL);
        vkFreeMemory(device, g_model_system.instanceMemory, NULL);
    }

    create_vulkan_buffer(newSize,
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                         &g_model_system.instanceBuffer,
                         &g_model_system.instanceMemory);

    g_model_system.instanceBufferSize = newSize;
    g_model_system.instanceCapacity = capacity;
}

void create_model_descriptors(void)
{
    VkDescriptorSetLayoutBinding bindings[2] = {0};

    // 0: Camera UBO
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // 1: Instance SSBO
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;
    vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &modelDescriptorSetLayout);

    VkDescriptorPoolSize poolSizes[2] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
    };
    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1;
    vkCreateDescriptorPool(device, &poolInfo, NULL, &modelDescriptorPool);

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = modelDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &modelDescriptorSetLayout;
    vkAllocateDescriptorSets(device, &allocInfo, &modelDescriptorSet);

    // Initial camera UBO write
    VkDescriptorBufferInfo camInfo = { cameraUBOBuffer, 0, sizeof(CameraUBO) };
    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = modelDescriptorSet;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &camInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
}

void update_model_descriptor(void)
{
    if (g_model_system.instanceBuffer == VK_NULL_HANDLE) return;

    VkDescriptorBufferInfo instInfo = {
        g_model_system.instanceBuffer,
        0,
        g_model_system.instanceBufferSize
    };

    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = modelDescriptorSet;
    write.dstBinding = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &instInfo;

    vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
}

void init_model_system(void)
{
    create_model_pipeline();

    g_model_system.models = NULL;
    g_model_system.modelCount = 0;
    g_model_system.modelCapacity = 0;

    g_model_system.instances = NULL;
    g_model_system.instanceCount = 0;
    g_model_system.instanceCapacity = 256;           // reasonable starting size
    g_model_system.instances = malloc(g_model_system.instanceCapacity * sizeof(InstanceData));

    g_model_system.instanceBuffer = VK_NULL_HANDLE;
    g_model_system.instanceMemory = VK_NULL_HANDLE;
    g_model_system.instanceBufferSize = 0;

    create_instance_buffer(g_model_system.instanceCapacity);

    // Add many instances
    for (int i = 0; i < 200; i++) {
        float transform[16] = {0}; // fill with your TRS matrix
        matrix_identity(transform);
        transform[12] = (float)(i % 20) * 2.0f - 20.0f;
        transform[13] = 0.0f;
        transform[14] = (float)(i / 20) * 2.0f - 10.0f;

        float color[4] = {0.2f + (i%5)*0.2f, 0.6f, 0.8f, 1.0f};
        add_model_instance("../../meshes/cs_goddess_statue_opt.glb", transform, color);
    }

    update_model_instances();
    update_model_descriptor();
}

Model* find_or_load_model(const char* glb_path)
{
    // === 1. First, check if we already loaded this model ===
    for (uint32_t i = 0; i < g_model_system.modelCount; ++i) {
        if (strcmp(g_model_system.models[i].name, glb_path) == 0) {
            return &g_model_system.models[i];   // Already loaded → reuse
        }
    }

    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, glb_path, &data);
    if (result != cgltf_result_success) {
        printf("Failed to parse glb: %s\n", glb_path);
        return NULL;
    }
    result = cgltf_load_buffers(&options, data, glb_path);
    if (result != cgltf_result_success) {
        printf("Failed to load buffers for glb: %s\n", glb_path);
        cgltf_free(data);
        return NULL;
    }
    if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
        printf("No mesh/primitive in glb: %s\n", glb_path);
        cgltf_free(data);
        return NULL;
    }

    cgltf_primitive* prim = &data->meshes[0].primitives[0];

    float* positions = NULL;
    float* normals = NULL;
    float* uvs = NULL;
    cgltf_size pos_count = 0, norm_count = 0, uv_count = 0;

    for (cgltf_size i = 0; i < prim->attributes_count; i++) {
        cgltf_attribute* attr = &prim->attributes[i];
        if (attr->type == cgltf_attribute_type_position) {
            pos_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
            positions = (float*)malloc(pos_count * sizeof(float));
            cgltf_accessor_unpack_floats(attr->data, positions, pos_count);
        }
        else if (attr->type == cgltf_attribute_type_normal) {
            norm_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
            normals = (float*)malloc(norm_count * sizeof(float));
            cgltf_accessor_unpack_floats(attr->data, normals, norm_count);
        }
        else if (attr->type == cgltf_attribute_type_texcoord) {
            uv_count = cgltf_accessor_unpack_floats(attr->data, NULL, 0);
            uvs = (float*)malloc(uv_count * sizeof(float));
            cgltf_accessor_unpack_floats(attr->data, uvs, uv_count);
        }
    }

    uint32_t* indices = NULL;
    cgltf_size index_count = 0;
    if (prim->indices) {
        index_count = prim->indices->count;
        indices = (uint32_t*)malloc(index_count * sizeof(uint32_t));
        cgltf_accessor_unpack_indices(prim->indices, indices, sizeof(uint32_t), index_count);
    }

    uint32_t vertexCount = (uint32_t)(pos_count / 3);
    Vertex3D* vertices = (Vertex3D*)malloc(vertexCount * sizeof(Vertex3D));

    for (uint32_t i = 0; i < vertexCount; i++) {
        vertices[i].x = positions[i*3 + 0];
        vertices[i].y = positions[i*3 + 1];
        vertices[i].z = positions[i*3 + 2];
        vertices[i].nx = normals ? normals[i*3 + 0] : 0.0f;
        vertices[i].ny = normals ? normals[i*3 + 1] : 0.0f;
        vertices[i].nz = normals ? normals[i*3 + 2] : 1.0f;
        vertices[i].u = uvs ? uvs[i*2 + 0] : 0.0f;
        vertices[i].v = uvs ? uvs[i*2 + 1] : 0.0f;
    }

    // Create Vulkan buffers for the mesh
    Mesh mesh = {0};
    VkDeviceSize vsize = vertexCount * sizeof(Vertex3D);

    create_vulkan_buffer(vsize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         &mesh.vertexBuffer, &mesh.vertexMemory);

    void* data_ptr;
    vkMapMemory(device, mesh.vertexMemory, 0, vsize, 0, &data_ptr);
    memcpy(data_ptr, vertices, vsize);
    vkUnmapMemory(device, mesh.vertexMemory);

    if (index_count > 0) {
        VkDeviceSize isize = index_count * sizeof(uint32_t);
        create_vulkan_buffer(isize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             &mesh.indexBuffer, &mesh.indexMemory);

        vkMapMemory(device, mesh.indexMemory, 0, isize, 0, &data_ptr);
        memcpy(data_ptr, indices, isize);
        vkUnmapMemory(device, mesh.indexMemory);
        mesh.indexCount = (uint32_t)index_count;
    }
    mesh.vertexCount = vertexCount;

    // === Grow the models array if needed ===
    if (g_model_system.modelCount >= g_model_system.modelCapacity) {
        g_model_system.modelCapacity = g_model_system.modelCapacity ? g_model_system.modelCapacity * 2 : 8;
        g_model_system.models = realloc(g_model_system.models,
                                        g_model_system.modelCapacity * sizeof(Model));
    }

    // Store the new Model
    Model* model = &g_model_system.models[g_model_system.modelCount++];
    model->name = _strdup(glb_path);           // store full path for simplicity
    model->mesh = mesh;                       // copy the Mesh struct
    model->instanceOffset = 0;                // we'll improve this later
    model->instanceCount = 0;

    // Cleanup temporary CPU data
    free(vertices);
    free(indices);
    free(positions);
    free(normals);
    free(uvs);
    cgltf_free(data);

    printf("Loaded model: %s (%u verts, %u indices)\n", 
           glb_path, mesh.vertexCount, mesh.indexCount);

    return model;
}

void add_model_instance(const char* model_name, const float transform[16], const float color[4])
{
    // Load (or find) the model ONLY ONCE
    static Model* cached_model = NULL;
    static const char* last_name = NULL;

    if (cached_model == NULL || strcmp(model_name, last_name) != 0) {
        cached_model = find_or_load_model(model_name);
        last_name = model_name;               // safe because path is constant string
    }

    if (!cached_model) return;

    // Grow instance array if needed
    if (g_model_system.instanceCount >= g_model_system.instanceCapacity) {
        g_model_system.instanceCapacity *= 2;
        if (g_model_system.instanceCapacity == 0) g_model_system.instanceCapacity = 256;
        g_model_system.instances = realloc(g_model_system.instances,
                                           g_model_system.instanceCapacity * sizeof(InstanceData));
    }

    uint32_t idx = g_model_system.instanceCount++;
    InstanceData* inst = &g_model_system.instances[idx];

    memcpy(inst->model, transform, 16 * sizeof(float));
    memcpy(inst->color, color, 4 * sizeof(float));

    cached_model->instanceCount++;   // bookkeeping
}

void update_model_instances(void)
{
    if (g_model_system.instanceCount == 0) return;

    VkDeviceSize needed = (VkDeviceSize)g_model_system.instanceCount * sizeof(InstanceData);
    if (needed > g_model_system.instanceBufferSize) {
        create_instance_buffer(g_model_system.instanceCount * 2);
        update_model_descriptor();
    }

    void* data;
    vkMapMemory(device, g_model_system.instanceMemory, 0, needed, 0, &data);
    memcpy(data, g_model_system.instances, needed);
    vkUnmapMemory(device, g_model_system.instanceMemory);
}

void draw_models(VkCommandBuffer cmd)
{
    if (g_model_system.modelCount == 0 || g_model_system.instanceCount == 0) return;

    update_model_instances();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            modelPipelineLayout, 0, 1, &modelDescriptorSet, 0, NULL);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);

    // For now we only support ONE model (the first one). We'll improve this soon.
    Model* m = &g_model_system.models[0];
    if (m->mesh.vertexBuffer == VK_NULL_HANDLE) return;

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m->mesh.vertexBuffer, &offset);

    if (m->mesh.indexCount > 0) {
        vkCmdBindIndexBuffer(cmd, m->mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, m->mesh.indexCount, g_model_system.instanceCount, 0, 0, 0);
    } else {
        vkCmdDraw(cmd, m->mesh.vertexCount, g_model_system.instanceCount, 0, 0);
    }
}

void cleanup_model_system(void)
{
    // Destroy all loaded models' meshes
    for (uint32_t i = 0; i < g_model_system.modelCount; ++i) {
        Mesh* m = &g_model_system.models[i].mesh;
        vkDestroyBuffer(device, m->vertexBuffer, NULL);
        vkFreeMemory(device, m->vertexMemory, NULL);
        if (m->indexCount > 0) {
            vkDestroyBuffer(device, m->indexBuffer, NULL);
            vkFreeMemory(device, m->indexMemory, NULL);
        }
        free(g_model_system.models[i].name);
    }

    free(g_model_system.models);
    free(g_model_system.instances);

    if (g_model_system.instanceBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, g_model_system.instanceBuffer, NULL);
        vkFreeMemory(device, g_model_system.instanceMemory, NULL);
    }

    // Reset everything
    g_model_system = (ModelSystem){0};
}