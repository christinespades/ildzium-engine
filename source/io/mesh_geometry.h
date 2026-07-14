#pragma once

typedef struct {
    uint32_t indexCount;
    uint32_t firstIndex;
    int32_t  vertexOffset;
    VkBuffer indexBuffer;  // If you use a unified global buffer, these might be global handles
    VkBuffer vertexBuffer;
} MeshGeometry;