#include "gtest/gtest.h"
#include <unordered_map>
#include <htsSharedLibrary.h>
#include <htsPluginAPI.h>
#include <htsLogInterface.h>

TEST(SharedLibrary, Test) {
	std::unordered_map<size_t, std::string> thread_map_;
	auto find = thread_map_.find(89203);

	hts::SharedLibrary lib;
	EXPECT_FALSE(lib.Load("notExist.dll"));
	EXPECT_TRUE(lib.Load("htsPluginFactory"));

	EXPECT_TRUE(lib.IsLoaded());
	EXPECT_TRUE(lib.GetSymbolAddress("notExist") == nullptr);
	EXPECT_TRUE(lib.GetSymbolAddress("GetPluginClassFactory") != nullptr);

	lib.Unload();
	EXPECT_FALSE(lib.IsLoaded());
}


TEST(htsPluginClassAPI, Test_IhtsPluginClassFactory) {
	hts::SharedLibrary lib;
	EXPECT_TRUE(lib.Load("htsPluginFactory"));

	IhtsPluginClassFactory* factory = ::GetPluginClassFactory();
	EXPECT_NE(factory, nullptr);
	EXPECT_TRUE(EnablehtsPluginFactoryLog("D:/out/htsFactory.log"));
}

class ITest : public IhtsPluginClass
{
public:
	virtual ~ITest(){};
	virtual int Add(int a, int b) = 0;
};
DEFINE_INTERFACE(ITest);

class TestAdd : public ITest
{
public:
	virtual ~TestAdd() {};
	virtual int Add(int a, int b) override
	{
		return a + b;
	}
public:
	const char name[5]{ "ABCD" };
};

TEST(htsPluginClassAPI, TestCreate_UserClass) {
	USE_LOGGER("");
	EXPECT_TRUE(m_logger);
	m_logger->EnterThread("main");

	bool registed = IhtsPluginClassFactory::RegistClass<ITest, TestAdd>();
	EXPECT_TRUE(registed);

	auto add1 = IhtsPluginClassFactory::CreateClass<ITest>("TestAdd", "first");
	auto add2 = IhtsPluginClassFactory::CreateClass<ITest>("TestAdd", "second");
	auto add3 = IhtsPluginClassFactory::CreateClass<ITest>("TestAdd", "first");
	EXPECT_NE(add1, nullptr);
	EXPECT_NE(add2, nullptr);
	EXPECT_NE(add3, nullptr);
	EXPECT_NE(add1, add2);
	EXPECT_EQ(add1, add3);

	EXPECT_EQ(add1->Add(4, 6), 10);

	add3.reset();
	EXPECT_EQ(add1->Add(4, 6), 10);

	void* buffer = malloc(sizeof(TestAdd));
	auto add = CreateClass<TestAdd>(buffer, sizeof(TestAdd));
	EXPECT_EQ(add->Add(4, 6), 10);
	add->IhtsPluginClass::~IhtsPluginClass();
}

TEST(htsLogAPI, Test_LogFmt) {
	USE_LOGGER("");
	EXPECT_TRUE(m_logger);
	m_logger->EnterThread("main");

	// 无法检查输出，但是如下的代码能编译通过就行
	LogFmtI("simple log");
	LogFmtI("{}+{}={}", 2, 3, 5);
	m_logger->LeaveThread();
}
