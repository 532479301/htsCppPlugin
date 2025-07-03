htsplugin_log4cplus是将log4cplus封装成htsplugin的log插件，当前版本基于log4cplus-2.1.2。

对log4cplus的修改：
1. Logger::shutdown ()函数内增加了是否已经销毁DefaultContext的判断，若已经销毁则不做任何事情