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
			//my_mutex2.lock();//实际工程中两个锁不一定挨着，可能他们需要保护不同的数据共享块
			//my_mutex1.lock();
			std::lock(my_mutex1, my_mutex2);
			msgRecvQueue.push_back(i);
			my_mutex2.unlock();
			my_mutex1.unlock();
		}
		return;
	}

	bool outMsgLULProc(int& command)
	{
		//std::lock_guard<std::mutex> LockGuard(my_mutex);//lock_guard<mutex>类只有拷贝构造函数
		//并且使用lock_guard后不能再使用lock()和unlock()

		/*my_mutex1.lock();
		my_mutex2.lock();*/

		std::lock(my_mutex1, my_mutex2);

		std::lock_guard<std::mutex> LockGuard1(my_mutex1,std::adopt_lock);
		std::lock_guard<std::mutex> LockGuard2(my_mutex2,std::adopt_lock);
		
		//因为下方对共享数据有操作，所以进行lock()
		if (!msgRecvQueue.empty())
		{
			//消息队列非空
			command = msgRecvQueue.front();//返回第一个元素
			msgRecvQueue.pop_front();//移除第一个元素
			//my_mutex.unlock();//不要忘记分支内的unlock()
			my_mutex1.unlock();
			my_mutex2.unlock();
			return true;
		}
		//my_mutex.unlock();//不要忘记分支内的unlock()
		my_mutex1.unlock();
		my_mutex2.unlock();
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
	std::list<int> msgRecvQueue;//模拟消息队列，代表玩家命令
	std::mutex my_mutex1;//创建了一个互斥量
	std::mutex my_mutex2;//创建另一个互斥量
};


int main()
{
	A obj;
	std::thread myOutMsgObj(&A::outMsgRecvQueue, std::ref(obj));//传入引用，保证线程中使用的是同一个对象的数据
	std::thread myInMsgObj(&A::inMsgRecvQueue, std::ref(obj));//传入引用，保证线程中使用的是同一个对象的数据
	myOutMsgObj.join();
	myInMsgObj.join();

	//保护共享数据，某个线程操作时用代码把共享数据锁住，其他想操作共享数据的线程必须等待解锁
	// 
	//一、互斥量(mutex)的基本概念
	//互斥量是一个对象，理解成一把锁，多个线程尝试用lock()成员函数来加锁，只有一个线程能够锁定成功(成功的标志为lock()函数返回)【如果没锁成功，那么流程会卡在lock()的执行上，直到锁定成功】
	//使用流程：在使用共享数据前加锁，使用完成后解锁
	//互斥量的使用要小心，保护数据不多也不少，少了达不到保护效果，多了影响效率

	//二、互斥量的用法
	//需要包含头文件mutex #include <mutex>
	//2.1 lock(),unlock()
		//步骤：先lock(),操作共享数据，再unlock()
		//使用规则：lock()与unlock()要成对使用，有lock()必然要有unlock()，且lock()与unlock()的调用次数对称 [n次lock()必须对应n次unlock()]
	//2.2 std::lock_guard类模板
	//有lock()无unlock()的问题非常难排查，所以C++引入了一个叫std::lock_guard的类模板(自动帮你unlock，类似于智能指针)
	//std::lock_guard会直接取代lock()和unlock():也就是说，用了std::lock_guard之后就不能使用lock()和unlock()
		//std::lock_guard类模板构造时调用lock()，析构时调用unlock()---作用域锁--->通过作用域的调整实现提前unlock
	//std::lock_guard的缺点:①无法便捷地控制unlock时机②多了一层包装，浪费性能

	//三、死锁
	//3.1 死锁演示
	//比如有两把锁(死锁问题是由两个或以上的互斥量产生的)金锁JinLock和银锁YinLock，两个线程A和B，(1)线程A执行的时候，A先锁金锁，把金锁lock()成功，然后他尝试去lock银锁；(2)此时由于系统调度，出现上下文切换，线程B执行了，B先锁银锁，恰好银锁还没有被lock，线程B就能把银锁lock()成功，然后线程B再尝试去lock金锁；(3)I.线程A拿不到银锁流程走不下去，后面的JinLock.unlock()无法执行，所以金锁解不开;II.线程B拿不到金锁，流程走不下去，后面的YinLock.unlock()无法执行; 大家都在等待，此时，死锁就产生了
	//3.2 死锁的一般解决方案
		//只要保证这两个互斥量上锁的顺序一致就不会死锁
	//3.3 std::lock()函数模板
	//作用：一次性锁住两个或两个以上的互斥量(至少两个，没有数量上限，1个不行)，故用于处理多个互斥量的时候才出场
	//不存在因为多个线程中锁的顺序导致死锁的风险
	//如果互斥量中有一个没锁住，std::lock()就会等待，直到所有互斥量都锁住了才会继续向下执行
		//在无法实现全部互斥量都锁定时，std::lock()会将他自己已锁的互斥量解锁 ———— 要么互斥量全锁住，要么互斥量全没锁
	//std::lock(my_mutex1, my_mutex2);
	//3.4 std::lock_guard的std::adopt_lock参数
	//结合使用std::lock的统一上锁和std::lock_guard类模板的自动解锁
	/*
	std::adopt_lock参数告诉模板类构造但不lock(构造函数的重载)，所以需要在构造std::lock_guard类对象之前使用std::lock()锁住
		std::lock(my_mutex1, my_mutex2);
		std::lock_guard<std::mutex> LockGuard1(my_mutex1,std::adopt_lock);
		std::lock_guard<std::mutex> LockGuard2(my_mutex2,std::adopt_lock);
	*/
	

	std::cout << " Hello " << std::endl;
	return 0;
}