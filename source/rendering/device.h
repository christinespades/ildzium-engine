#pragma once
#ifndef __EMSCRIPTEN__
	extern VkPhysicalDevice physicalDevice;
	extern VkDevice vk_device;
	extern VkQueue graphicsQueue;
	extern uint32_t queueFamilyIndex;

	void pick_physical_device(VkInstance instance, VkSurfaceKHR surface);
	void create_logical_device();
#else
	WGPUQueue queue;
#endif