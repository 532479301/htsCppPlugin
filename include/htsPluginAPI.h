// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#pragma once
#include <filesystem>
#include <string>
#include <cassert>

#ifdef HTS_PLUGINCLASSAPI_EXPORTS
#define HTS_PLUGINCLASSAPI __declspec(dllexport)
#else
#define HTS_PLUGINCLASSAPI __declspec(dllimport)
#pragma comment(lib,"htsPluginFactory.lib")
#endif

class IhtsPluginClassFactory;
extern "C" HTS_PLUGINCLASSAPI IhtsPluginClassFactory * GetPluginClassFactory();		// the htsClassFactory only export this function

/**
* The root class for all interface class used for plugin class.
* Any interface class must single inheritance from this class directly or indirectly.
*/
class IhtsPluginClass
{
public:
	virtual ~IhtsPluginClass() {};

public:
	std::string instanceName()
	{
		return m_instanceName;
	}
	const char* className()
	{
		return m_pClassName;
	}
	const char* classNameSimple()
	{
		for (const char* pos = m_pClassName + strlen(m_pClassName) - 1; pos >= m_pClassName; pos--)
		{
			if (*pos == ':')
				return pos + 1;
		}
		return m_pClassName;
	}

protected:
	virtual void SetInstanceName(const char* name)
	{
		if (name != nullptr)
			m_instanceName = name;
	};

private:
	friend class htsClassInfo;
	friend class htsClassFactory;
	const char* m_pInterfaceName;
	const char* m_pClassName;
	std::string m_instanceName;
};

typedef IhtsPluginClass* (*CreateClassFunc)(void* pBuffer, size_t size);
typedef void (*DeleteClassFunc)(IhtsPluginClass*);
#define DEFINE_INTERFACE(interfaceClassName) typedef std::shared_ptr<interfaceClassName> interfaceClassName##Ptr

template<typename T>
std::enable_if_t<std::is_class<T>::value, const char*> ClassName()
{
	const char* m_name(typeid(T).name());
	if (strncmp(m_name, "class ", 6) == 0)
		return m_name + 6;
	else
		return nullptr;
}

template <typename T>
std::enable_if_t<std::is_class<T>::value, T*> CreateClass(void* pBuffer, size_t size)
{
	assert(size >= sizeof(T));
	return new(pBuffer) T();
}

class IhtsPluginClassFactory
{
public:
	virtual ~IhtsPluginClassFactory() {};

public:
	template<typename Interface, typename T>
	static std::enable_if_t<std::is_base_of_v<Interface, T>, bool> RegistClass()
	{
		return GetPluginClassFactory()->RegistClass(typeid(Interface), typeid(T), sizeof(T), 
			[](void* pBuffer, size_t size)->IhtsPluginClass* {
				assert(size >= sizeof(T));
				return dynamic_cast<IhtsPluginClass*>(new(pBuffer) T);
			}
		);
	}

	template<typename Interface, typename T>
	static std::enable_if_t<std::is_base_of_v<Interface, T>, void> UnregistClass()
	{
		GetPluginClassFactory()->UnregistClass(typeid(Interface), typeid(T));
	}

	template<typename Interface>
	static std::shared_ptr<Interface> CreateClass(const char* pClassName = nullptr, const char* pInstanceName = "")
	{
		IhtsPluginClass* pbase = GetPluginClassFactory()->CreateClass(typeid(Interface), pClassName, pInstanceName);
		if (pbase != nullptr)
		{
			return std::shared_ptr<Interface>(dynamic_cast<Interface*>(pbase),
				[](Interface* ptr) {GetPluginClassFactory()->ReleaseClass(dynamic_cast<IhtsPluginClass*>(ptr)); });
		}
		else
		{
			return std::shared_ptr<Interface>();
		}
	}

public:
	virtual bool LoadDLL(std::filesystem::path pPluginPath) = 0;
	virtual void UnloadDLL(std::filesystem::path pPluginPath) = 0;
	virtual bool EnableLog(const char* pClassName, const char* loggerName, const char* config) = 0;

private:
	//virtual bool RegistClass(const char* pInterfaceName, const char* pClassName, CreateClassFunc pCreateFn, DeleteClassFunc pDeleteFn) = 0;
	virtual bool RegistClass(const type_info& interfaceType, const type_info& classType, size_t size, CreateClassFunc pCreateFn) = 0;
	virtual void UnregistClass(const type_info& interfaceType, const type_info& classType) = 0;
	virtual IhtsPluginClass* CreateClass(const type_info& interfaceType, const char* pClassName, const char* pInstanceName = "") = 0;
	virtual void ReleaseClass(IhtsPluginClass* pClass) = 0;
};

inline bool EnablehtsPluginFactoryLog(std::filesystem::path logFilePath)
{
#if defined(_DEBUG) && defined(_MSC_VER)
	static const char htsPluginFactoryLogConfig[] = R"(
		{
			"pattern" : "[%%D %%T.%%e] [%%t : %%N] [%%l] %%v", 
			"logger" :
			[
				{"name": "htsPluginFactory", "async": true, "sink": 
					[
						{"name": "msvc_sink_mt", "params": {}},
						{"name": "rotating_file_sink_mt", "params": {"file_name" : "%s", "max_size" : 10000000, "max_files" : 3}}
					]
				}
			],
			"thread_pool" : {"queue_size" : 512, "thread_count" : 1}
		})";
#else
	static const char htsPluginFactoryLogConfig[] = R"(
		{"logger" :
			[
				{"name": "htsPluginFactory", "async": true, "sink": 
					[
						{"name": "rotating_file_sink_mt", "params": {"file_name" : "%s", "max_size" : 10000000, "max_files" : 3}}
					]
				}
			]
		})";
#endif

	char buffer[_MAX_PATH + sizeof(htsPluginFactoryLogConfig)];
	sprintf_s(buffer, htsPluginFactoryLogConfig, logFilePath.string().c_str());
	return GetPluginClassFactory()->EnableLog("htsplugin_SPDLogger\\SPDLogger", "htsPluginFactory", buffer);
}