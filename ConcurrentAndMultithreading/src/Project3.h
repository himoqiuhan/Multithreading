#pragma once

#include <iostream>
#include <thread>


//为了验证陷阱2的问题，自己创建一个类
class Temp
{
public:
	mutable int m_i;
	//类型转换构造函数，将一个int转化为一个类
	Temp(int i) :m_i(i) { std::cout << "[Temp::Temp(int i)构造函数执行]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl; }
	Temp(const Temp& other) :m_i(other.m_i) { std::cout << "[Temp::Temp(const Temp &other)拷贝构造函数执行]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl; }
	~Temp() { std::cout << "[Temp::~Temp()析构函数执行]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl; }

	//成员函数指针做线程函数
	void thread_work(int num)
	{
		std::cout << "[子线程thread_work执行]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl;
	}

	//用类对象创建线程(仿函数)
	void operator()(int num)
	{
		std::cout << "[子线程operator()执行]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl;
	}
};

void Test(const Temp& temp)
{
	std::cout << &temp << std::endl;
	return;
}
//------陷阱2的验证------//end

//1.1 要避免的陷阱1---detach引发的内存回收问题
//void MyPrint(const int &i, char *pmybuf)
	//①i虽然是引用型的参数，但实际上是值传递，即便主线程detach了子线程，子线程中的i仍然是安全的
	//②子线程中的指针变量pmybuf指向的内存空间就是主线程中传入参数MyBuffer指向的内存空间，所以指针在detach子线程时，绝对会有问题
//所以使用detach时不推荐传入引用型参数，不要传入指针变量
void MyPrint(const int i, const std::string& pmybuf)
{
	std::cout << i << std::endl;
	std::cout << pmybuf << std::endl;
	return;
}

//使用Thread2来查看各个构造函数的执行时机
void Thread2(const Temp& obj)
{
	obj.m_i = 100;
	std::cout << "线程2函数参数地址为" << &obj << " ";
	std::cout << "线程2线程id为" << std::this_thread::get_id() << std::endl;
}

//传递智能指针
void Thread3(std::unique_ptr<int> ptr)
{
	std::cout << "线程3函数参数地址为" << ptr << " ";
	std::cout << "线程3线程id为" << std::this_thread::get_id() << std::endl;
}

int main()
{
	//给线程函数传参，第一个参数构造函数第一个参数为线程函数，后续参数为线程函数参数

	//一、传递临时对象作为线程参数
	// detach带来的两个陷阱
	//1.1 要避免的陷阱1---内存回收问题
	//1.2 要避免的陷阱2---主线程向线程函数传参时隐式转换(类的隐式构造)发生的时机
		//存在危险：const char类型的数组MyBuffer在被传递给mytobj线程时需要发生const char到const string的隐式转换，但是可能在转换前，MyBuffer就因为主线程执行完毕而被回收，那么实际转换的内存可能是已经被回收的内存
		//只要用临时构造的类对象作为参数传递给线程，那么就能在主线程执行完毕前把类参数传递给子线程，是安全的
	//1.3 总结
		//a.如果传递简单类型参数，建议使用值传递
		//b.如果传递类对象，
			//i.需要避免隐式类型转换：传递实际的对象，或者在函数参数中构造临时对象
			//ii.函数参数需要接收引用类型的变量，否则可能还会多构造一次
		//结论：不使用detach，只是用join，这样就不存在局部变量失效导致线程对内存非法引用的问题

	//二、进一步探究临时对象作为函数参数的问题
	//2.1 线程id的概念：每个线程实际上都对应着一个数字，不同的线程其线程id不同
		//线程id可以通过C++标准库里的函数来获取: std::this_thread::get_id();
	//2.2 临时对象构造时机抓捕

	//三、传递类对象、智能指针作为线程参数
	//3.1 传递类对象进入子线程，即使传入引用但实际上传递的也是主线程内对象的一个拷贝对象，所以子线程内对成员进行改变不会影响到主线程中的对象
	//3.2 传递能够在子线程中对主线程产生影响的变量：使用std::ref(<变量>...)
		//a.主线程中传入的如果不是std::ref变量，则子线程函数参数必须声明为const
		//b.若主线程中传入std::ref变量，则任意
	//3.3 传递智能指针
		//a.unique指针需要通过std::move(<ptr>)传递

	//四、用成员函数指针做线程函数


	int MyVar = 1;
	int& MyVary = MyVar;
	char MyBuffer[] = "this is a buffer";

	//1.1 要避免的陷阱2
	//std::thread mytobj(MyPrint, MyVar, MyBuffer);
	//std::thread mytobj(MyPrint, MyVar, std::string(MyBuffer));//解决陷阱2---传参前使用MyBuffer创建一个临时对象作为参数传递给子线程


	//begin------陷阱2的验证------
	//为了更直观地看陷阱2存在的调用情况，使用自己创建的类进行验证
	//std::thread TestThread(Test, MyVar);//在打印Hello之后执行构造函数，存在风险；
	//std::thread TestThread(Test, Temp(MyVar));//在打印Hello之前执行了类型转换构造函数(创建临时对象)、拷贝构造函数(实际传参)、析构函数(该行执行完毕后回收临时对象)，安全传递类对象
	//TestThread.detach();
	//------陷阱2的验证------end

	//mytobj.join();
	//mytobj.detach();

	////begin------2.2 临时对象构造时机抓捕------
	//std::cout << "主线程线程id为" << std::this_thread::get_id() << std::endl;
	//std::cout << "------使用隐式类型转换------" << std::endl;
	//std::thread thread2A(Thread2, MyVar);
	////使用隐式类型转换时，对象是在子线程中进行构造的，使用detach时就会出现问题
	//std::cout << "------使用临时对象------" << std::endl;
	//std::thread thread2B(Thread2, Temp(MyVar));
	////使用临时对象时，会先在主线程中构造一个临时对象(类型转换构造函数)，然后再在主线程中调用拷贝构造函数(在主线程中进行的创建)，最后析构临时对象并分线程执行后续操作，那么detach后在子线程中调用就不会有安全性问题

	//thread2A.join();
	//thread2B.join();
	////------end

	//begin------3.1-3.2向子线程中传递可变对象------
	Temp obj(10);
	std::thread thread3a(Thread2, obj);
	thread3a.join();
	std::cout << obj.m_i << std::endl;//打印10
	std::thread thread3b(Thread2, std::ref(obj));
	thread3b.join();
	std::cout << obj.m_i << std::endl;//打印100
	//------end

	//begin------3.3传递智能指针------
	std::unique_ptr<int> ptr(new int(100));
	std::thread thread3(Thread3, std::move(ptr));//利用std::move传递
	thread3.join();//传入的智能指针指向地址来自主线程，使用detach又可能会有问题
	//------end

	//begin------使用类成员函数指针做线程函数------
	Temp obj(10);
	std::thread Thread4A(&Temp::thread_work, obj, 15);//需要传入成员函数指针、对象，之后才是传入函数的参数
	//传入的对象会在主线程中执行一次拷贝构造，所以可以使用detach
	Thread4A.detach();
	std::thread Thread4B(&Temp::thread_work, std::ref(obj), 15);//同理可使用std::ref来传递引用进入子线程，但是这时就不能使用detach了
	Thread4B.join();
	//------end

	std::cout << "Hello" << std::endl;
	return 0;
}
