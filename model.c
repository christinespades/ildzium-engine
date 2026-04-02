#define CGLTF_IMPLEMENTATION
#define CGLTF_MESHOPT_DECODE 1
#define MESHOPTIMIZER_IMPLEMENTATION
#include "cgltf.h"
#include "model.h"
#include "renderer.h" 
#include "shaders.h"
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
    if (g_model.instanceBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, g_model.instanceBuffer, NULL);
        vkFreeMemory(device, g_model.instanceMemory, NULL);
    }

    create_vulkan_buffer(newSize,
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                         &g_model.instanceBuffer,
                         &g_model.instanceMemory);

    g_model.instanceBufferSize = newSize;
    g_model.instanceCapacity = capacity;
}

void init_model_system(void)
{
    create_model_pipeline();

    g_model.instanceCapacity = 64;
    g_model.instances = malloc(g_model.instanceCapacity * sizeof(InstanceData));
    g_model.instanceCount = 0;
    g_model.instanceBuffer = VK_NULL_HANDLE;
    g_model.instanceMemory = VK_NULL_HANDLE;

    create_instance_buffer(g_model.instanceCapacity);
}

void load_model(const char* glb_path)
{
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, glb_path, &data);
    if (result != cgltf_result_success) {
        printf("Failed to parse glb: %s\n", glb_path);
        return;
    }
    result = cgltf_load_buffers(&options, data, glb_path);
    if (result != cgltf_result_success) {
        printf("Failed to load buffers for glb: %s\n", glb_path);
        cgltf_free(data);
        return;
    }
    if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
        printf("No mesh/primitive in glb: %s\n", glb_path);
        cgltf_free(data);
        return;
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

    // Create mesh buffers
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

    // Store mesh
    g_model.meshes = malloc(sizeof(Mesh));
    *g_model.meshes = mesh;
    g_model.meshCount = 1;

    // Cleanup
    free(vertices);
    free(indices);
    free(positions);
    free(normals);
    free(uvs);
    cgltf_free(data);

    printf("Loaded model: %s (%u verts, %u indices)\n", glb_path, vertexCount, (uint32_t)index_count);
}

void add_model_instance(const float* modelMatrix, const float* color)
{
    if (g_model.instanceCount >= g_model.instanceCapacity) {
        uint32_t newCap = g_model.instanceCapacity * 2;
        InstanceData* newInst = realloc(g_model.instances, newCap * sizeof(InstanceData));
        if (!newInst) return;

        g_model.instances = newInst;
        g_model.instanceCapacity = newCap;

        create_instance_buffer(newCap);
        update_model_descriptor();   // declared in renderer.h
    }

    InstanceData* inst = &g_model.instances[g_model.instanceCount++];
    memcpy(inst->model, modelMatrix, 16 * sizeof(float));

    if (color) {
        memcpy(inst->color, color, 4 * sizeof(float));
    } else {
        inst->color[0] = inst->color[1] = inst->color[2] = 1.0f;
        inst->color[3] = 1.0f;
    }
}

void update_model_instances(void)
{
    if (g_model.instanceCount == 0) return;

    VkDeviceSize needed = (VkDeviceSize)g_model.instanceCount * sizeof(InstanceData);
    if (needed > g_model.instanceBufferSize) {
        create_instance_buffer(g_model.instanceCount * 2);
        update_model_descriptor();
    }

    void* data;
    vkMapMemory(device, g_model.instanceMemory, 0, needed, 0, &data);
    memcpy(data, g_model.instances, needed);
    vkUnmapMemory(device, g_model.instanceMemory);
}

void draw_models(VkCommandBuffer cmd)
{
    if (g_model.meshCount == 0 || g_model.instanceCount == 0) return;

    update_model_instances();

    // Bind descriptor set (camera UBO + instance SSBO)
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            modelPipelineLayout, 0, 1, &modelDescriptorSet, 0, NULL);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);

    Mesh* m = &g_model.meshes[0];
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m->vertexBuffer, &offset);

    if (m->indexCount > 0) {
        vkCmdBindIndexBuffer(cmd, m->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, m->indexCount, g_model.instanceCount, 0, 0, 0);
    } else {
        vkCmdDraw(cmd, m->vertexCount, g_model.instanceCount, 0, 0);
    }
}

void destroy_model(Model* model)
{
    if (!model || model->meshCount == 0) return;

    // Destroy mesh buffers
    for (uint32_t i = 0; i < model->meshCount; ++i) {
        Mesh* m = &model->meshes[i];
        vkDestroyBuffer(device, m->vertexBuffer, NULL);
        vkFreeMemory(device, m->vertexMemory, NULL);
        if (m->indexCount > 0) {
            vkDestroyBuffer(device, m->indexBuffer, NULL);
            vkFreeMemory(device, m->indexMemory, NULL);
        }
    }
    free(model->meshes);

    // Destroy instance buffer
    if (model->instanceBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, model->instanceBuffer, NULL);
        vkFreeMemory(device, model->instanceMemory, NULL);
    }

    free(model->instances);

    model->meshes = NULL;
    model->instances = NULL;
    model->instanceBuffer = VK_NULL_HANDLE;
    model->instanceMemory = VK_NULL_HANDLE;
    model->meshCount = 0;
    model->instanceCount = 0;
    model->instanceCapacity = 0;
}

void cleanup_model_system(void)
{
    destroy_model(&g_model);
}