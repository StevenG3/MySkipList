#ifndef __CONFIGS_H__
#define __CONFIGS_H__

#include "non_copyable.h"
#include <iostream>
#include <mutex>

// 怎么记录Redis的一些配置情况
// 1. 读取配置文件
// 2. 读取命令行参数
// 3. 读取环境变量
// 4. 读取默认值

// 该类完成从配置文件、命令行参数、环境变量、默认值中读取配置信息
// 该类的实例对象是全局唯一的
// 该类的实例对象是线程安全的
// 该类的实例对象是懒汉式的
// 该类的实例对象是单例模式的

class Configs: NonCopyable {
public:
	// 获取该类的实例对象
	static Configs* GetInstance();

	// 从配置文件中读取配置信息
	void LoadConfigFile(const char *filename);

	// 从命令行参数中读取配置信息
	void LoadCmdLine(int argc, char *argv[]);

	// 从环境变量中读取配置信息
	void LoadEnv();

	// 从默认值中读取配置信息
	void LoadDefault();

	// 获取配置信息
	int GetPort() const;
	int GetThreadNum() const;
	int GetMaxConn() const;
	int GetMaxEvents() const;
	int GetTimeout() const;
	int GetLogWrite() const;
	int GetLogStdout() const;
	int GetLogLevel() const;
	int GetCloseLog() const;
	int GetActorModel() const;
	int GetMaxMemory() const;
	int SetMaxMemory(int max_memory);
private:
	static Configs* configs_ptr_;
	static std::mutex mutex_;

	int max_memory_;
};

// 在类外初始化静态成员变量
Configs* Configs::configs_ptr_ = nullptr;
std::mutex Configs::mutex_;

// 单例模式实现
// 线程安全的单例模式实现
Configs* Configs::GetInstance() {
	// 双重锁模式
	if (configs_ptr_ == nullptr) {
		// 加锁
		std::lock_guard<std::mutex> lock(mutex_);
		if (configs_ptr_ == nullptr) {
			configs_ptr_ = new Configs();
		}
		// 解锁
	}
	return configs_ptr_;
}

// 从配置文件中读取配置信息
void Configs::LoadConfigFile(const char *filename) {
	// TODO
}

// 从命令行参数中读取配置信息
void Configs::LoadCmdLine(int argc, char *argv[]) {
	// TODO
}

// 从环境变量中读取配置信息
void Configs::LoadEnv() {
	// TODO
}

// 从默认值中读取配置信息
void Configs::LoadDefault() {
	// TODO
}

// 获取配置信息
int Configs::GetPort() const {
	// TODO
	return 0;
}

int Configs::GetThreadNum() const {
	// TODO
	return 0;
}

int Configs::GetMaxConn() const {
	// TODO
	return 0;
}

int Configs::GetMaxEvents() const {
	// TODO
	return 0;
}

int Configs::GetTimeout() const {
	// TODO
	return 0;
}

int Configs::GetLogWrite() const {
	// TODO
	return 0;
}

int Configs::GetLogStdout() const {
	// TODO
	return 0;
}

int Configs::GetLogLevel() const {
	// TODO
	return 0;
}

int Configs::GetCloseLog() const {
	// TODO
	return 0;
}

int Configs::GetActorModel() const {
	// TODO
	return 0;
}

int Configs::GetMaxMemory() const {
	// TODO
	return max_memory_;
}

int Configs::SetMaxMemory(int max_memory) {
	if(max_memory < 0) {
		return 1;
	}

	max_memory_ = max_memory;
	return 0;
}

#endif // __CONFIGS_H__