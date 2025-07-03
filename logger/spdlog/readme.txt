htsplugin_spdlog是将spdlog封装成htsplugin的log插件，当前版本基于spdlog 1.15.3。

对spdlog的修改：
1. 增加对线程名称的记录，以方便了解线程是谁。线程开始和结束的地方分别调用EnterThread和LeaveThread即可在该线程的log里面输出线程名字（名字是EnterThread时给的）
2. 异步log时，修复退出main函数后的log丢失问题（退出main后还有log？是的，比如在全局类的析构里写log）
3. 不使用fmt库，仅在C++20时支持format