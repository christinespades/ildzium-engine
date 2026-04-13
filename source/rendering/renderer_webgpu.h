#pragma once

typedef enum {
    GPU_STATE_NOT_READY = 0,
    GPU_STATE_ADAPTER_OK,
    GPU_STATE_DEVICE_OK,
    GPU_STATE_READY
} GPUState;

extern GPUState gpu_state;
