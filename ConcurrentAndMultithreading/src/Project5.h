#pragma once

#include <iostream>
#include <thread>
#include <mutex>

#include <vector>
#include <list>

//用成员函数作为线程函数
class A
{
public:
	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue()执行，插入一个元素" << i << std::endl;

			std::unique_lock<std::mutex> UniqueLock1(my_mutex1, std::try_to_lock);
			if (UniqueLock1.owns_lock())
			{
				//拿到了锁
				msgRecvQueue.push_back(i);
			}
			else
			{
				std::cout << "---inMsgRecvQueue没拿到锁---" << i <<std::endl;
			}
		}
		return;
	}

	bool outMsgLULProc(int& command)
	{
		std::unique_lock<std::mutex> LockGuard1(my_mutex1);
	
		/*my_mutex1.lock();
		std::unique_lock<std::mutex> LockGuard1(my_mutex1, std::adopt_lock);*/

		std::chrono::milliseconds duration(200000);
		std::this_thread::sleep_for(duration);//线程休息一段时长

		if (!msgRecvQueue.empty())
		{
			command = msgRecvQueue.front();
			msgRecvQueue.pop_front();
			return true;
		}
		return false;
	}

	//把数据从消息队列取出的线程
	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; i++)
		{
			bool result = outMsgLULProc(command);
			if (result)
			{
				std::cout << "outMsgRecvQueue()执行，取出一个元素" << command << std::endl;
			}
			else
			{
				std::cout << "outMsgRecvQueue()执行，目前消息队列中为空" << i << std::endl;
			}
		}
		std::cout << std::endl;
	}

private:
	std::list<int> msgRecvQueue;
	std::mutex my_mutex1;
	std::mutex my_mutex2;
};

int main()
{
	A obj;
	std::thread myOutMsgObj(&A::outMsgRecvQueue, std::ref(obj));
	std::thread myInMsgObj(&A::inMsgRecvQueue, std::ref(obj));
	myOutMsgObj.join();
	myInMsgObj.join();


	//一、unique_lock取代lock_guard
	//unique_lock是个类模板，工作中，一般推荐使用lock_guard(lock_guard取代mutex.lock和mutex.unlock)
	//unique_lock比lock_guard更灵活，但效率更差内存、占用更高

	//二、unique_lock构造函数的第二参数(灵活性的体现)
	//2.1 std::adopt_lock
	//表明该互斥量已经被lock了，则在构造函数中不再需要lock
	//前提是在构造前将互斥量lock
	//2.2 std::try_to_lock
	//尝试用mutex.lock()去锁定该mutex，但如果没有lock成功，内部的lock也会返回，线程中的程序继续执行，并不会阻塞在那里
	//前提是使用try_to_lock前不能手动lock
	//2.3 std::defer_lock
	//初始化一个没有lock的uniqu_lock---使用unique_lock包裹mutex，但没有lock
	//前提是使用defer_lock前不能手动lock

	//三、unique_lock的成员函数
	//3.1 lock()
	//加锁
		//注意是对unique_lock的类对象调用，而不是mutex
		/*
		std::unique_lock<std::mutex> UniqueLock(my_mutex,std::defer_lock);
		UniqueLock.lock();
		*/
	//3.2 unlock()
	//解锁
		//注意是对unique_lock的类对象调用，而不是mutex
	//在线程执行中，锁内中间有一段非共享数据代码要处理，但不希望这个锁锁住这一块，就可以调用unlock和lock来灵活锁定与解锁
		/*
		std::unique_lock<std::mutex> UniqueLock(my_mutex,std::defer_lock);
		UniqueLock.lock();
		//...处理共享数据...
		UniqueLock.Unlock();
		//...处理非共享数据...
		UniqueLock.lock();
		//...再次处理共享数据...
		//...
		*/
	//注：unique_lock的析构函数在执行unlock前会判断mutex是否被lock，如果已经手动提前解锁，则析构函数中就不会再执行unlock
	//3.3 try_lock()
	//尝试加锁，返回布尔值，加锁成功返回true，失败返回false
	//类似于try_to_lock参数的作用，lock失败也不会阻塞
	//3.4 release()
	//返回它所管理的mutex对象指针，并释放所有权，也就是说，让这个mutex和unique_lock对象不再有关系
	//使用std::mutex*数据类型来接收函数返回值
	//unlock()和release()的区别：
		//unlock()是解锁，对应的mutex还包裹在unique_lock对象内;
		//release()是释放，返回原始的mutex指针，将内部的mutex与unique_lock对象内的成员分离，且mutex保持原状态(原本是lock就是lock，原本是unlock就是unlock)
	//3.5 总结
	//a.为什么需要有unlock():因为lock所著的代码段越少，执行越快，整个程序效率越高
	//b.有人也把锁锁住的代码的多少成为锁的粒度，粒度一般用粗细来描述
		//锁住的代码少，粒度细，执行效率高；锁住的代码多，粒度粗，执行效率低
		//要选择合适的粒度对代码进行保护

	//四、unique_lock所有权的传递(mutex)
	//UniqueLock可以将自己对my_mutex的所有权转移给其他unique_lock对象
	//注意：unique_lock所有权可以转移，但不能复制
	//1.1 方法1--使用std::move()
	/*
	std::unique_lock<std::mutex> UniqueLock(my_mutex);//此时UniqueLock对my_mutex具有所有权
	std:unique_lock<std::mutex> NewUniqueLock(std::move(UniqueLock));
	*/
	//1.2 方法2--函数return std::unique_lock<std::mutex>
	/*
	例如：
	class A
	{
		private:
		std::mutex my_mutex;
		public:
		std::unique_lock<std::mutex> rtn_unique_lock()
		{
			std::unique_lock<std::mutex> temp(my_mutex);
			return temp;
		}

		void test()
		{
			std::unique_lock<std::mutex> UniqueLock = rtn_unique_lock();//会调用unique_lock的移动构造函数，将临时对象实例化，所有权转移
		}
	};
	*/

	return 0;
}