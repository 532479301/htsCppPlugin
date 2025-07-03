// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#include <map>
#include <cassert>
#include "htsLogInterface.h"
#include "htsPluginFactory.h"

IhtsPluginClass* htsClassInfo::CreateClass(const char* pInstanceName)
{
	assert(pInstanceName != nullptr);

	std::unique_lock<std::mutex> lock(m_mutex);
	auto ni = instances.find(pInstanceName);
	if (ni != instances.end())
	{
		if (ni->second->ptr != nullptr)
		{
			// 实例已经存在
			++ni->second->count;
			++counter;
			return ni->second->ptr;
		}
	}
	else
	{
		instances[pInstanceName] = std::make_shared<ClassInstance>();
	}

	auto ci = instances[pInstanceName];
	++ci->count;	// 阻止其他线程释放此实例
	lock.unlock();

	std::call_once(ci->flag, [this, &ci, pInstanceName] {
		IhtsPluginClass* pt = pCreateFn(malloc(objectSize), objectSize);
		pt->SetInstanceName(pInstanceName);
		pt->m_pInterfaceName = interfaceName.c_str();
		pt->m_pClassName = className.c_str();

		ci->ptr = pt;
		});

	lock.lock();
	ni = instances.find(pInstanceName);
	if (ni != instances.end())
	{
		if (ni->second->ptr != nullptr)
		{
			++counter;
			return ni->second->ptr;
		}
		else
		{
			// 创建新实例失败
			instances.erase(pInstanceName);
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

void htsClassInfo::ReleaseClass(IhtsPluginClass* pClass)
{
	auto instanceName = pClass->instanceName();

	std::unique_lock<std::mutex> lock(m_mutex);
	auto ni = instances.find(instanceName);
	if (ni != instances.end())
	{
		assert(ni->second->count > 0);
		assert(ni->second->ptr == pClass);
		// 待优化：下面这两个count是否可以只用一个？
		--ni->second->count;
		--counter;
		if (ni->second->count == 0 && !instanceName.empty())	// 不再有人使用。说明：对于无名（即instanceName为空）的实例，是当作单例使用的，即使当前没人使用了，也并不删除。
		{
			instances.erase(instanceName);
			lock.unlock();
			pClass->~IhtsPluginClass();
			free(pClass);
		}
		return;
	}

	auto m_logger = htsClassFactory::GetFactory().m_logger;
	LogI("%p is not created by factory or already released!", pClass);
}

void htsClassInfo::DebugMemoryLeak()
{
	auto m_logger = htsClassFactory::GetFactory().m_logger;
	LogI("Detect memory leak for (%s : %s):", interfaceName.c_str(), className.c_str());

	for (auto& T : instances)
	{
		if (!T.first.empty() || T.second->count > 0)
			LogI("Class instance: %p (%s) not release, counter is %d", T.second->ptr, T.first.c_str(), T.second->count);
		T.second->ptr->~IhtsPluginClass();
		free(T.second->ptr);
		T.second->ptr = nullptr;
	}
}

void htsClassInfo::UnregistClass()
{
	// 调用UnregistClass的时候，本ClassInfo已经处于不再被管理的状态，因此没有其他线程能访问到了
	if (instances.size() > 0)
	{
		auto m_logger = htsClassFactory::GetFactory().m_logger;
		LogI("%d instance is still alived when unregisted %s : %s", instances.size(), interfaceName, className);
		DebugMemoryLeak();
	}
}
