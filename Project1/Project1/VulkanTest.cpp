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
		glfwInit();//��ʼ�� glfw ��
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//��ȥ���� OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//������������
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//��߼ӱ��⣬��ָ����ʾ�������һ��������GL��أ����ù�
	}

	void initVulkan()
	{
		createInstance();
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

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);//����ʹ����glfw��ܣ���ȡ�ÿ����Ҫ���õ���չ

		std::vector<const char*> requiredExtensions;

		for (uint32_t i = 0; i < glfwExtensionCount; i++)
		{
			requiredExtensions.emplace_back(glfwExtensions[i]);
		}

		requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);//mac ƽ̨����Ҫ���ø���չ
		
		std::cout << "required extension:\n";
		for (const auto& requiredEXT : requiredExtensions)
		{
			std::cout << '\t' << requiredEXT << '\n';
		}

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

		createInfo.enabledLayerCount = 0;//��ʱ����

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
		vkDestroyInstance(instance, nullptr);//�˳���ʱ��Ҫ����instance������֮ǰ����������������������vk��Դ
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