//

#include <htsSharedLibrary.h>
#include <htsPluginAPI.h>
#include <htsLogInterface.h>
#include <htsProfile.h>

class ITest : public IhtsPluginClass
{
public:
	virtual ~ITest() {};
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

void Run(const int threadId, const int count, const bool sameName)
{
	printf("Run test: threadId = %d, count = %d \n", threadId, count);
	int failed = 0;
	int64_t sum = 0;
	char name[16];
	for (int i = 0; i < count; i++)
	{
		auto index = sameName ? i : (threadId * count + i + 1);
		sprintf_s(name, "Name_%d", index);
		auto add = IhtsPluginClassFactory::CreateClass<ITest>("TestAdd", name);
		if (add)
			sum += add->Add(4, 6);
		else
			++failed;
	}
	if (sum != count * 10)
	{
		printf("Wrong! Create class failed %d\n", failed);
	}
}

int main()
{
	int testCount = 10000;

	auto factory = GetPluginClassFactory();
	EnablehtsPluginFactoryLog("d:\\log\\pluginClassFactory.log");
	if (IhtsPluginClassFactory::RegistClass<ITest, TestAdd>())
	{
		auto res = benchmark(2, 5, Run, { testCount, testCount }, { false, true });
		printf("benchmark(2, 5, Run, { testCount, testCount }, { false, false }) = %f\n", res);
		res = benchmark(6, 5, Run, testCount, false);
		printf("benchmark(6, 5, Run, testCount, false) = %f\n", res);
		printf("Profile finished!\n");
	}
	else
	{
		printf("注册TestAdd失败！\n");
	}
}