#pragma once

#define VK_USE_PLATEFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <map>
#include <optional>
#include <bitset>
#include <set>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"//Vulkan ��׼��չ
};

#ifdef NDEBUG 
const bool enableValidationLayer = false;
#else
const bool enableValidationLayer = true;
#endif

class HelloTriangleApplication
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback//��̬��Ա�ص�������vk��������������Ǵ���ʵ�֡�
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,//Vulkan ������������������Ϣ�ļ���
		VkDebugUtilsMessageTypeFlagsEXT messageType,//Vulkan ������������������Ϣ�ķ���Ŀ¼����
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,//Vulkan ����ͨ������ṹ���������������Ϣ�ľ�������
		void* pUserData//��ʱû�ã����Ը�������һЩ�Զ����������Ϣ
	);

public:
	void run();

private:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

private:
	void initWindow();

	void initVulkan();
	void createInstance();	
	std::vector<const char*> getRequiredExtension();
	bool checkValidationLayerSupport();

	void setupDebugMessenger();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance
		, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger
		, const VkAllocationCallbacks* pAllocator);

	void createSurface();

	void pickPhysicalDevice();
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createLogicalDevice();

	void mainLoop();

	void cleanup();

private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
};