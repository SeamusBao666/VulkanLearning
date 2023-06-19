#include "VulkanTest.h"

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
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)//�������Warning����ʹ�ӡ
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
	glfwInit();//��ʼ�� glfw ��
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//��ȥ���� OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//������������
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//��߼ӱ��⣬��ָ����ʾ�������һ��������GL��أ����ù�
}

void HelloTriangleApplication::initVulkan()
{
	createInstance();
	setupDebugMessenger();//Validation Layer ���ú���Ҫ��ʼ���������ֻ�ǵ���������һ��Layer��û��ʵ�ʹ��ܵ�ʹ��
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
}

void HelloTriangleApplication::setupDebugMessenger()
{
	if (!enableValidationLayer)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};//��Ҫ�ȴ���һ��Messenger��������VK���������Ƿ���Ϣ
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |//����Warning��Log��Error��Verbose
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |//����PxrUnreal��LogTemp��LogHMD
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;//��Ҫָ��һ���ص���������Ϣ��ͨ������ص��������ظ����ǿ�����
	createInfo.pUserData = nullptr;//���ǿ����ߴ������Ϣ����ʱ����

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)//����Messenger
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkResult HelloTriangleApplication::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");//��ȡ����չ�������ӿڲ��ҵ���
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
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//����Messenger
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
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());//ͨ��Instance��ѯ��ǰӲ����Vulkan����֧�ֵ�API Layer

	std::cout << "available layers" << std::endl;
	for (const auto& layerProperties : availableLayers)
	{
		std::cout << '\t' << layerProperties.layerName << std::endl;

	}

	for (const char* layerName : validationLayers)
	{
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)//��� Validation Layer �Ƿ�֧�֣����֧�ֵĻ��ͷ���true
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
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);//����ʹ����glfw��ܣ���ȡ�ÿ����Ҫ���õ���չ

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);//mac ƽ̨����Ҫ���ø���չ
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
	VkApplicationInfo appInfo{};//Ӧ����Ϣ
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";//����
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);//�Զ���汾
	appInfo.pEngineName = "No Engine";//�Զ����������֣�����������������棿
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);//�Զ�������汾
	appInfo.apiVersion = VK_API_VERSION_1_0;//ʹ�õ�vk sdk �İ汾��

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;//���������app��Ϣ

	uint32_t supportedExtensionCount = 0;//call twice ���
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);//�Ȳ�����
	std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);//������ռ�
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());//��������

	std::cout << "supported extension:\n";//����������ǰinstance֧����Щ��չ����Щ��չ�Ƕ������Կ������еģ�����ͬһ�汾sdk�����ǲ�ͬƽ̨������Ŀ��ܲ�һ��
	for (const auto& extension : supportedExtensions)
	{
		std::cout << '\t' << extension.extensionName << '\n';
	}

	auto requiredExtensions = getRequiredExtension();

	std::cout << "enabled extension:\n";
	std::vector<const char*> enabledExtensions;
	for (const auto& supportedEXT : supportedExtensions)//��������˲�֧�ֵ���չ�ᱨ������VK_ERROR_EXTENSION_NOT_PRESENT����VK_ERROR_INCOMPATIBLE_DRIVER 
	{
		for (const auto& requiredEXT : requiredExtensions)//����������Ҫ��ѯ������Ҫ����չ���豸����ַ��֧�֣������֧����ô���ǲ�Ӧ�������������߷�����ƽ̨
		{
			if (strcmp(supportedEXT.extensionName, requiredEXT) == 0)
			{
				enabledExtensions.emplace_back(requiredEXT);
				if (requiredEXT == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
				{
					createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;//mac ƽ̨�����ø���չ����instance��Ҫ�����Ӧ��flag
				}
				std::cout << '\t' << requiredEXT << '\n';
				break;
			}
		}
	}

	createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();//����ȷ������instance������չ������
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();//���õ���չ

	if (enableValidationLayer)//Debug ģʽ�¿���Validation Layer
	{
		if (checkValidationLayerSupport())//����Ƿ�֧��Validation Layer
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());//�����õ�Layer ��������������
			createInfo.ppEnabledLayerNames = validationLayers.data();//����ʵ����ʱ��������Layer
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

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);//����instance����instance��vk sdk �ṩ��������������Ӧ�ú�����(���߽�runtime)֮��ͨ�ŵ�
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
	vkDestroyDevice(device, nullptr);
	if (enableValidationLayer)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);//�������ע�͵������߼������ܵ�Validation Layer �ı�����Ϣ�����˳�Ӧ�õ�ʱ��
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);//�˳���ʱ��Ҫ����instance������֮ǰ����������������������vk��Դ
	glfwDestroyWindow(window);
	glfwTerminate();
}
