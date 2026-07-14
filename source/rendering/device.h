#pragma once
#ifndef __EMSCRIPTEN__
	extern VkPhysicalDevice physicalDevice;
	extern VkDevice vk_device;
	extern VkQueue graphicsQueue;
	extern uint32_t queueFamilyIndex;
	extern VkSampleCountFlagBits msaaSamples;
	
	void pick_physical_device(VkInstance instance, VkSurfaceKHR surface);
	void create_logical_device();
#else

	typedef enum {
	    GPU_STATE_NOT_READY = 0,
	    GPU_STATE_ADAPTER_OK,
	    GPU_STATE_DEVICE_OK,
	    GPU_STATE_READY
	} GPUState;

	extern GPUState gpu_state;
	extern WGPUQueue queue;
#endif