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
	)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)//�������Warning����ʹ�ӡ
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
		glfwInit();//��ʼ�� glfw ��
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//��ȥ���� OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//������������
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//��߼ӱ��⣬��ָ����ʾ�������һ��������GL��أ����ù�
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();//Validation Layer ���ú���Ҫ��ʼ���������ֻ�ǵ���������һ��Layer��û��ʵ�ʹ��ܵ�ʹ��
	}

	void setupDebugMessenger()
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

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance
		, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugUtilsMessengerEXT* pDebugMessenger)
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

	void DestroyDebugUtilsMessengerEXT(VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger
		, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//����Messenger
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

	std::vector<const char*> getRequiredExtension()
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

	void createInstance()
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
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);//�������ע�͵������߼������ܵ�Validation Layer �ı�����Ϣ�����˳�Ӧ�õ�ʱ��
		}
		vkDestroyInstance(instance, nullptr);//�˳���ʱ��Ҫ����instance������֮ǰ����������������������vk��Դ
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