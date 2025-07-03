// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#include <atomic>
#include <map>
#include <list>
#include <mutex>
#include <htsSharedLibrary.h>
#include "htsPluginAPI.h"
#include "htsLogInterface.h"

struct ClassInstance
{
	IhtsPluginClass* ptr = nullptr;
	int count = 0;
	std::once_flag flag;
};

class htsClassInfo
{
public:
	std::string interfaceName;
	std::string className;
	std::filesystem::path dllName;
	CreateClassFunc pCreateFn;
	size_t objectSize;
	int counter = 0;
	std::map<std::string, std::shared_ptr<ClassInstance>> instances;

	IhtsPluginClass* CreateClass(const char* pInstanceName);
	void ReleaseClass(IhtsPluginClass* pClass);
	void DebugMemoryLeak();
	void UnregistClass();

private:
	std::mutex m_mutex;
};
typedef std::shared_ptr<htsClassInfo> ClassInfoPtr;

struct PluginLoadInfo
{
	PluginLoadInfo& operator =(PluginLoadInfo& val)
	{		
		this->dll = val.dll;
		this->instance = val.instance;
		this->counter = val.counter.load();
		return *this;
	}
	std::shared_ptr<hts::SharedLibrary> dll = std::make_shared<hts::SharedLibrary>();
	std::atomic<int> counter = 0;
	std::map<void*, ClassInfoPtr> instance;
};

class htsClassFactory : public IhtsPluginClassFactory
{
public:
	virtual ~htsClassFactory();

	static htsClassFactory& GetFactory()
	{
		static htsClassFactory the_factory;
		return the_factory;
	}

public:
	virtual bool LoadDLL(std::filesystem::path pPluginPath) override;
	virtual void UnloadDLL(std::filesystem::path pPluginPath) override;
	virtual bool RegistClass(const type_info& interfaceType, const type_info& classType, size_t size, CreateClassFunc pCreateFn) override;
	virtual void UnregistClass(const type_info& interfaceType, const type_info& classType) override;
	virtual IhtsPluginClass* CreateClass(const type_info& interfaceType, const char* pClassName, const char* pInstanceName = "") override;
	virtual void ReleaseClass(IhtsPluginClass* pClass) override;
	virtual bool EnableLog(const char* pClassName, const char* loggerName, const char* config) override;

private:
	htsClassFactory();

private:
	std::filesystem::path GetPluginPath(std::filesystem::path);
	bool IsClassRegisted(const char* pInterfaceName, const char* pClassName);
	bool IsPluginLoaded(std::filesystem::path& pDllName);
	void UnregistClass(std::filesystem::path pDllName);
	ClassInfoPtr FindClass(const char* pInterfaceName, const char* pClassName);

private:
	typedef std::map<std::string, ClassInfoPtr> ClassMap;

	std::mutex m_mutex;
	std::map<std::string, ClassMap> m_InterfaceMap;

	std::mutex m_mutexPlugin;
	std::map<std::filesystem::path, PluginLoadInfo> m_pluginMap;

	std::filesystem::path m_pluginDir;

	friend class htsClassInfo;
	std::string m_loggerClassName;
	hts::ILoggerPtr m_logger;
};
