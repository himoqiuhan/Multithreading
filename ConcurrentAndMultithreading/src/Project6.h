#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <list>

#define Use_call_once true

#if Use_call_once
std::once_flag g_flag;//这是个系统定义的标记，同样需要是全局变量
#else
std::mutex resource_mutex;//创建全局mutex，让所有线程访问到的互斥量是同一个
#endif
class Singleton
{
private:
	Singleton(){}//私有化构造函数
	~Singleton() { std::cout << "Destructor!" << std::endl; }

	static void CreateInstance()//只调用一次的函数
	{
		m_instance = new Singleton();
		static Reclaim rec;
	}

private:
	static Singleton* m_instance;//静态成员指针，指向自己的静态实例

public:
	Singleton(const Singleton& other) = delete;//覆盖拷贝构造函数，确保Singleton的唯一性

	static Singleton* GetInstance()//返回内部实例的指针(或者返回引用)
	{
#if Use_call_once
		std::call_once(g_flag, CreateInstance);//两个线程同时执行到这里时，其中一线程要等另外一个线程执行完毕CreateInstance函数，改变g_flag标记后，再判断g_flag标记情况；如果标记为已执行，则不会再执行CreateInstance函数
#else
		if (m_instance == nullptr)//双重检查，提高效率，防止无意义地构造unique_lock浪费性能
		//需要使用lock锁定的原因：
			//如果m_instance != nullptr条件成立，一定表示m_instance已经被new过了
			//如果m_instance == nullptr条件成立，不一定能保证m_instance还没有被new，可能是一个线程正准备执行m_instance = new Singleton();这一行代码，而另一个线程恰好完成了if条件的判断
		{
			std::unique_lock<std::mutex> my_mutex(resource_mutex);//自动加锁
			if (m_instance == nullptr)
			{
				m_instance = new Singleton();
				static Reclaim rec;//通过一个嵌套类来释放实例的指针
				//不能在析构里直接调用delete，因为自始至终都没有创建实体类，所以不会在外部调用析构函数；因此需要创建一个嵌套类，通过嵌套类的析构来调用singleton的析构函数并释放内存
			}
		}
#endif
		return m_instance;
	}

	//使用嵌套类回收singleton的内存
	class Reclaim
	{
	public:
		~Reclaim()
		{
			if (Singleton::m_instance)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}
	};

};
Singleton* Singleton::m_instance = nullptr;//初始化静态成员实例指针

void MyThread()
{
	std::cout << std::this_thread::get_id() << "线程开始了" << std::endl;
	Singleton* MySingleton = Singleton::GetInstance();//此处会出问题
	std::cout << std::this_thread::get_id() << "线程结束了" << std::endl;
	return;
}

int main()
{
	//一、设计模式概论
	//“设计模式”：代码的一些非常规写法，程序灵活，维护方便，但是别人接管、阅读代码很困难。使用“设计模式”理念写出来的代码很晦涩
	//设计模式来源于程序员应付特别大的项目的时，把项目的开发经验、模块划分经验总结整理成设计模式(先有的开发需求，后进行的理论总结)

	//二、单例设计模式
	//单例设计模式使用的频率比较高
	//单例(Singleton)：整个项目中，有某个或者某些特殊的类，属于该类的对象，只能创建一个
	//单例类的写法
		//1.私有化构造函数
		//2.指向一个类实例的静态成员变量
		//3.Get静态函数返回该静态指针
		//4.main函数中通过Get函数获取该静态指针，或者通过调用接口静态函数来执行函数

	//三、单例设计模式共享数据问题分析、解决
	//理想情况是能够在主线程中，创建线程前初始化单例内部的数据
	//面临的问题：需要在我们自己创建的线程(非主线程)中来创建单例类的对象，且其他线程可能使用这个单例类
	//我们可能会面临GetInstance()函数需要互斥的情况

	//虽然两个函数入口函数相同，但是这是两个线程，所以此处会有两条通路同时执行MyThread()
	std::thread MyThread1(MyThread);
	std::thread MyThread2(MyThread);
	MyThread1.join();
	MyThread2.join();


	//四、std::call_once()
	//C++11引入的函数，该函数的第二个参数是一个函数名method()，call_once的功能是保证函数method()即使在多线程下也只被调用一次
	//call_once具备互斥量能力，且比互斥量更高效
	//call_once需要与一个标记结合使用：std::once_flag
		//call_once就是通过这个标记来决定对应的函数method()是否执行，调用call_once()成功后，call_once()就把once_flag设置为已调用状态；后续再次调用call_once()，只要once_flag被设置为了“已调用”状态，那么对应的函数method()就不会再被执行了
	


	std::cout << "hello" << std::endl;
	return 0;
}