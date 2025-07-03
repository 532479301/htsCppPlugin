// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#include <cassert>
#include <htsPathHelper.h>
#include "htsPluginAPI.h"
#include "htsPluginFactory.h"

HTS_PLUGINCLASSAPI IhtsPluginClassFactory* GetPluginClassFactory()
{
	return &htsClassFactory::GetFactory();
}

htsClassFactory::htsClassFactory()
{
	m_pluginDir.assign(hts::PathHelper::GetPathFromFilePath(hts::PathHelper::GetDLLFileName()));
}

htsClassFactory::~htsClassFactory()
{
	LogI("htsPluginFactory is existing, destoring ...");
	std::lock_guard pluginLock(m_mutexPlugin);
	while (!m_pluginMap.empty())
	{
		auto T = m_pluginMap.begin();
		if (T->second.counter > 0)
		{
			LogI("NOT UnloadPlugin %s, counter is %d", T->first.string().c_str(), T->second.counter.load());
		}
		T->second.dll->Unload();
		m_pluginMap.erase(m_pluginMap.begin());
	}

	ClassInfoPtr log;
	std::unique_lock<std::mutex> lock(m_mutex);
	for (auto T : m_InterfaceMap)
	{
		while (T.second.size() > 0)
		{
			ClassInfoPtr ci = T.second.begin()->second;
			if (ci->className != m_loggerClassName)
			{
				T.second.erase(T.second.begin());
				lock.unlock();
				ci->DebugMemoryLeak();
				lock.lock();
			}
			else
			{
				log = ci;
				T.second.erase(T.second.begin());
			}
		}
	}
	LogI("htsPluginFactory existed");
	lock.unlock();
	m_logger.reset();
}

bool htsClassFactory::EnableLog(const char* pClassName, const char* loggerName, const char* config)
{
	if (m_logger)
		return true;

	m_logger = IhtsPluginClassFactory::CreateClass<hts::ILogger>(pClassName, loggerName);
	if (m_logger && m_logger->Config(config))
	{
		m_loggerClassName = m_logger->className();
		return true;
	}
	else
	{
		m_loggerClassName.clear();
		m_logger.reset();
		return false;
	}
}

ClassInfoPtr htsClassFactory::FindClass(const char* pInterfaceName, const char* pClassName)
{
	assert(pInterfaceName != nullptr);

	std::lock_guard<std::mutex> lock(m_mutex);

	auto it = m_InterfaceMap.find(pInterfaceName);
	if (it != m_InterfaceMap.end())
	{
		if (pClassName == nullptr || pClassName[0] == 0)
		{
			if (it->second.size() > 0)
				return it->second.begin()->second;
		}
		else
		{
			auto find = it->second.find(pClassName);
			if (find != it->second.end())
			{
				return find->second;
			}
		}
	}
	return ClassInfoPtr();
}

bool htsClassFactory::IsClassRegisted(const char* pInterfaceName, const char* pClassName)
{
	assert(pInterfaceName != nullptr);

	std::lock_guard<std::mutex> lock(m_mutex);

	auto it = m_InterfaceMap.find(pInterfaceName);
	if (it != m_InterfaceMap.end())
	{
		auto find = it->second.find(pClassName);
		return (find != it->second.end());
	}
	return false;
}

inline const char* ClassName(const char* name)
{
	if (strncmp(name, "class ", 6) == 0)
		return name + 6;
	else
		return nullptr;
}

bool htsClassFactory::RegistClass(const type_info& interfaceType, const type_info& classType, size_t size, CreateClassFunc pCreateFn)
{
	auto pInterfaceName = ClassName(interfaceType.name());
	auto pClassName = ClassName(classType.name());
	if (pInterfaceName == nullptr || pClassName == nullptr) return false;

	assert(size > sizeof(IhtsPluginClass));
	assert(pCreateFn != nullptr);

	std::lock_guard<std::mutex> lock(m_mutex);
	auto& classMap = m_InterfaceMap[pInterfaceName];
	auto find = classMap.find(pClassName);
	if (find == classMap.end())
	{
		ClassInfoPtr ci = std::make_shared<htsClassInfo>();
		ci->interfaceName = pInterfaceName;
		ci->className = pClassName;
		ci->dllName = hts::PathHelper::GetDLLFileName(pCreateFn);
		ci->objectSize = size;
		ci->pCreateFn = pCreateFn;
		ci->counter = 0;
		classMap[pClassName] = ci;

		LogI("%s: %s in %s registed.\n", pInterfaceName, pClassName, ci->dllName.string().c_str());
		return true;
	}
	else
	{
		LogI("%s: %s is already registed. Duplicated?\n", pInterfaceName, pClassName);
	}
	return false;
}

void htsClassFactory::UnregistClass(const type_info& interfaceType, const type_info& classType)
{
	auto pInterfaceName = ClassName(interfaceType.name());
	auto pClassName = ClassName(classType.name());

	std::unique_lock<std::mutex> lock(m_mutex);
	auto it = m_InterfaceMap.find(pInterfaceName);
	if (it != m_InterfaceMap.end())
	{
		auto find = it->second.find(pClassName);
		if (find != it->second.end())
		{
			ClassInfoPtr ci = find->second;
			it->second.erase(find);
			lock.unlock();
			ci->UnregistClass();
		}
	}
}

void htsClassFactory::UnregistClass(std::filesystem::path pDllName)
{
	std::list<ClassInfoPtr> classToUnregist;

	// 找出所有要取消注册的插件
	std::unique_lock<std::mutex> lock(m_mutex);
	for (auto T = m_InterfaceMap.begin(); T != m_InterfaceMap.end(); T++)
	{
		for (auto i = T->second.begin(); i != T->second.end();)
		{
			if (i->second->dllName.compare(pDllName) == 0)
			{
				classToUnregist.push_back(i->second);
				T->second.erase(i++);
			}
			else
				i++;
		}
	}
	lock.unlock();

	for (auto ci : classToUnregist)
	{
		LogI("%s: %s in %s unregisted.", ci->interfaceName, ci->className, pDllName.c_str());
		ci->UnregistClass();
	}
}

bool htsClassFactory::IsPluginLoaded(std::filesystem::path& pDllName)
{
	std::lock_guard<std::mutex> lock(m_mutexPlugin);
	auto find = m_pluginMap.find(pDllName);
	return (find != m_pluginMap.end());
}

IhtsPluginClass* htsClassFactory::CreateClass(const type_info& interfaceType, const char* pClassName, const char* pInstanceName)
{
	auto pInterfaceName = ClassName(interfaceType.name());

	std::string className;

	if (pClassName != nullptr)
	{
		className = pClassName;

		size_t pos = className.find("\\");
		if (pos != std::string::npos)
		{
			// DLL\Class
			std::string dll = className.substr(0, pos);
			LoadDLL(dll);
			pClassName = className.c_str() + pos + 1;
		}
	}

	ClassInfoPtr classInfo = FindClass(pInterfaceName, pClassName);
	if (classInfo)
	{
		return classInfo->CreateClass(pInstanceName);
	}
	LogI("Could not find registed class %s:%s, no instance created!", pInterfaceName, pClassName);
	return NULL;
}

void htsClassFactory::ReleaseClass(IhtsPluginClass* pClass)
{
	assert(pClass != nullptr);

	ClassInfoPtr classInfo = FindClass(pClass->m_pInterfaceName, pClass->m_pClassName);
	if (classInfo != nullptr)
	{
		LogI("ReleaseClass %s:%s, name: %s", pClass->m_pInterfaceName, pClass->m_pClassName, pClass->m_instanceName.c_str());
		classInfo->ReleaseClass(pClass);
	}
	else
	{
		LogI("ReleaseClass %s:%s, name: %s failed. Could not find class info!",
			pClass->m_pInterfaceName, pClass->m_pClassName, pClass->m_instanceName.c_str());
	}
}

std::filesystem::path htsClassFactory::GetPluginPath(std::filesystem::path path)
{
	if (path.is_relative())
	{
		std::filesystem::path ret = m_pluginDir;
		ret.append(path.string());
		return weakly_canonical(ret);	// 去掉路径中的"." ".."
	}
	else
		return path;
}

bool htsClassFactory::LoadDLL(std::filesystem::path path)
{
	auto fullPath = GetPluginPath(path);

	std::unique_lock<std::mutex> lock(m_mutexPlugin);
	auto find = m_pluginMap.find(fullPath);
	if (find != m_pluginMap.end())
	{
		// Already Loaded;
		assert(find->second.counter > 0);
		find->second.counter++;
		LogI("Load %s: already loaded. Reference count is: %d.", path.c_str(), find->second.counter.load());
		return true;
	}

	lock.unlock();
	PluginLoadInfo info;
	if (info.dll->Load(fullPath))
	{
		info.counter = 1;
		lock.lock();
		m_pluginMap[info.dll->GetPath()] = info;
		return true;
	}
	else
	{
		// 未加载过，且加载失败
		LogI("Load %s: not loaded. LoadLibrary error!", path.c_str());
		return false;
	}
}

void htsClassFactory::UnloadDLL(std::filesystem::path path)
{
	auto fullPath = GetPluginPath(path);

	std::unique_lock<std::mutex> lock(m_mutexPlugin);
	auto find = m_pluginMap.find(fullPath);
	if (find != m_pluginMap.end())
	{
		find->second.counter--;
		if (find->second.counter == 0)
		{
			auto dll = find->second.dll;	// 避免从map中删除时卸载dll
			m_pluginMap.erase(fullPath);
			lock.unlock();

			UnregistClass(fullPath);
			dll->Unload();
			LogI("Unload %s: done.", path.c_str());
		}
		else
		{
			LogI("Unload %s: can not unload it now. Reference count is: %d.\n", path.string().c_str(), find->second.counter.load());
		}
	}
	else
	{
		LogI("Unload %s: done. Not load before OR already unloaded.\n", path.string().c_str());
	}
}
