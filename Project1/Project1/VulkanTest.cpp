#include "VulkanTest.h"
#include <cstdint>
#include <limits>
#include <algorithm>

int main()
{
	HelloTriangleApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)//如果大于Warning级别就打印
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
	return VK_TRUE;
}

void HelloTriangleApplication::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void HelloTriangleApplication::initWindow()
{
	glfwInit();//初始化 glfw 库
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//不去创建 OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//不处理窗口缩放
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//宽高加标题，不指定显示器，最后一个参数跟GL相关，不用管
}

void HelloTriangleApplication::initVulkan()
{
	createInstance();
	setupDebugMessenger();//Validation Layer 启用后需要初始化，否则就只是单纯启动了一个Layer，没有实际功能的使用
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
}

void HelloTriangleApplication::setupDebugMessenger()
{
	if (!enableValidationLayer)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};//需要先创建一个Messenger，用来让VK驱动给我们发消息
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |//类似Warning，Log，Error，Verbose
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |//类似PxrUnreal，LogTemp，LogHMD
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;//需要指定一个回调函数，消息会通过这个回调函数返回给我们开发者
	createInfo.pUserData = nullptr;//我们开发者传入的信息，暂时无用

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)//创建Messenger
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkResult HelloTriangleApplication::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");//获取该扩展的新增接口并且调用
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void HelloTriangleApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//销毁Messenger
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

bool HelloTriangleApplication::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());//通过Instance查询当前硬件的Vulkan驱动支持的API Layer

	std::cout << "available layers" << std::endl;
	for (const auto& layerProperties : availableLayers)
	{
		std::cout << '\t' << layerProperties.layerName << std::endl;

	}

	for (const char* layerName : validationLayers)
	{
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)//检查 Validation Layer 是否支持，如果支持的话就返回true
			{
				std::cout << "validation layer available!" << std::endl;
				return true;
			}
		}
	}

	return false;
}

void HelloTriangleApplication::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface");
	}
}

void HelloTriangleApplication::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0)
	{
		physicalDevice = candidates.rbegin()->second;
	}
	else
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensionSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	return indices.isComplete() && extensionSupported && swapChainAdequate;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCount,nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCount,availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(),deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

int HelloTriangleApplication::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	std::cout << "device:" << deviceProperties.deviceName << std::endl;

	int score = 0;
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	score += deviceProperties.limits.maxImageDimension2D;
	if (!deviceFeatures.geometryShader)
	{
		return 0;
	}

	if (isDeviceSuitable(device))
	{
		score += 1000;
	}
}

HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::cout << "queueFamilies cout:" << queueFamilyCount << std::endl;

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		std::cout << "graphicsFamily[" << i << "] flags:" << std::bitset<sizeof(queueFamily.queueFlags) * 8>(queueFamily.queueFlags) << std::endl;
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);
		if (presentSupported)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

HelloTriangleApplication::SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,surface,&details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device,surface,&formatCount,nullptr);
	if (formatCount!=0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device,surface,&formatCount,details.formats.data());
	}
	
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface,&presentModeCount,nullptr);
	if (presentModeCount!=0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface,&presentModeCount,details.presentModes.data());
	}
	return details;
}

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
	{
		std::cout << "capabilities.currentExtent.width:" << capabilities.currentExtent.width
			<< "capabilities.currentExtent.height:" << capabilities.currentExtent.height << std::endl;
		return capabilities.currentExtent;
	}
	else 
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		std::cout << "glfwGetFramebufferSize.width:" << width
			<< "glfwGetFramebufferSize.height:" << height << std::endl;

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

void HelloTriangleApplication::createSwapChain()
{
	SwapChainSupportDetails swapchainSupport = querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);
	
	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.minImageCount > 0 && imageCount > swapchainSupport.capabilities.minImageCount)
	{
		imageCount = swapchainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily.value())
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//qq 音乐半透播放器
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}
	
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void HelloTriangleApplication::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void HelloTriangleApplication::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		std::cout << "uniqueQueueFamily:" << queueFamily << std::endl;
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) == VK_SUCCESS)
	{
		std::cout << "succeed to create logical device!" << std::endl;
	}
	else
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtension()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);//我们使用了glfw框架，获取该框架需要启用的扩展

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);//mac 平台上需要启用该扩展
	if (enableValidationLayer)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	std::cout << "required extension:\n";
	for (const auto& requiredEXT : extensions)
	{
		std::cout << '\t' << requiredEXT << '\n';
	}
	return extensions;
}

void HelloTriangleApplication::createInstance()
{
	VkApplicationInfo appInfo{};//应用信息
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";//标题
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);//自定义版本
	appInfo.pEngineName = "No Engine";//自定义引擎名字，比如起名叫虚幻引擎？
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);//自定义引擎版本
	appInfo.apiVersion = VK_API_VERSION_1_0;//使用的vk sdk 的版本号

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;//传入上面的app信息

	uint32_t supportedExtensionCount = 0;//call twice 风格
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);//先查数量
	std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);//再申请空间
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());//再拿数据

	std::cout << "supported extension:\n";//用来表明当前instance支持哪些扩展，这些扩展是定义在显卡驱动中的，基于同一版本sdk，但是不同平台查出来的可能不一样
	for (const auto& extension : supportedExtensions)
	{
		std::cout << '\t' << extension.extensionName << '\n';
	}

	auto requiredExtensions = getRequiredExtension();

	std::cout << "enabled extension:\n";
	std::vector<const char*> enabledExtensions;
	for (const auto& supportedEXT : supportedExtensions)//如果启用了不支持的扩展会报错，比如VK_ERROR_EXTENSION_NOT_PRESENT或者VK_ERROR_INCOMPATIBLE_DRIVER 
	{
		for (const auto& requiredEXT : requiredExtensions)//所以我们需要查询我们想要的扩展该设备到地址不支持，如果不支持那么我们不应该启用他，或者放弃该平台
		{
			if (strcmp(supportedEXT.extensionName, requiredEXT) == 0)
			{
				enabledExtensions.emplace_back(requiredEXT);
				if (requiredEXT == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
				{
					createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;//mac 平台上启用该扩展创建instance还要配合相应的flag
				}
				std::cout << '\t' << requiredEXT << '\n';
				break;
			}
		}
	}

	createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();//最终确定创建instance启用扩展的数量
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();//启用的扩展

	if (enableValidationLayer)//Debug 模式下开启Validation Layer
	{
		if (checkValidationLayerSupport())//检查是否支持Validation Layer
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());//将弃用的Layer 传入数量和名称
			createInfo.ppEnabledLayerNames = validationLayers.data();//创建实例的时候将启动该Layer
		}
		else
		{
			std::runtime_error("validation layers requested,but not available!");
		}
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);//创建instance，该instance是vk sdk 提供给我们用来进行应用和驱动(或者叫runtime)之间通信的
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance");
	}
}

void HelloTriangleApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void HelloTriangleApplication::cleanup()
{
	for (auto imageView : swapChainImageViews) 
	{
		vkDestroyImageView(device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	if (enableValidationLayer)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);//如果我们注释掉这行逻辑，会受到Validation Layer 的报错信息，在退出应用的时候
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);//退出的时候要销毁instance，不过之前得销毁我们所创建的所有vk资源
	glfwDestroyWindow(window);
	glfwTerminate();
}
