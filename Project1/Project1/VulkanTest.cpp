#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication
{
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

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);//我们使用了glfw框架，获取该框架需要启用的扩展

		std::vector<const char*> requiredExtensions;

		for (uint32_t i = 0; i < glfwExtensionCount; i++)
		{
			requiredExtensions.emplace_back(glfwExtensions[i]);
		}

		requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);//mac 平台上需要启用该扩展
		
		std::cout << "required extension:\n";
		for (const auto& requiredEXT : requiredExtensions)
		{
			std::cout << '\t' << requiredEXT << '\n';
		}

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

		createInfo.enabledLayerCount = 0;//暂时不管

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
		vkDestroyInstance(instance, nullptr);//退出的时候要销毁instance，不过之前得销毁我们所创建的所有vk资源
		glfwDestroyWindow(window);
		glfwTerminate();
	}
private:
	GLFWwindow* window;
	VkInstance instance;
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