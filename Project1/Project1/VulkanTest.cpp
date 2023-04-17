#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"//Vulkan 标准扩展
};

#ifdef NDEBUG 
const bool enableValidationLayer = false;
#else
const bool enableValidationLayer = true;
#endif

class HelloTriangleApplication
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback//静态成员回调函数，vk驱动调这个，我们处理实现。
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,//Vulkan 驱动告诉我们这条消息的级别
		VkDebugUtilsMessageTypeFlagsEXT messageType,//Vulkan 驱动告诉我们这条消息的分组目录类型
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,//Vulkan 驱动通过这个结构体告诉我们他的消息的具体内容
		void* pUserData//暂时没用，可以告诉我们一些自定义的数据信息
	)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)//如果大于Warning级别就打印
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}
		return VK_TRUE;
	}
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	void initWindow()
	{
		glfwInit();//初始化 glfw 库
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//不去创建 OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//不处理窗口缩放
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//宽高加标题，不指定显示器，最后一个参数跟GL相关，不用管
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();//Validation Layer 启用后需要初始化，否则就只是单纯启动了一个Layer，没有实际功能的使用
	}

	void setupDebugMessenger()
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

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance
		, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugUtilsMessengerEXT* pDebugMessenger)
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

	void DestroyDebugUtilsMessengerEXT(VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger
		, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//销毁Messenger
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	bool checkValidationLayerSupport()
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

	std::vector<const char*> getRequiredExtension()
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

	void createInstance()
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



	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		if (enableValidationLayer)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);//如果我们注释掉这行逻辑，会受到Validation Layer 的报错信息，在退出应用的时候
		}
		vkDestroyInstance(instance, nullptr);//退出的时候要销毁instance，不过之前得销毁我们所创建的所有vk资源
		glfwDestroyWindow(window);
		glfwTerminate();
	}
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
};

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