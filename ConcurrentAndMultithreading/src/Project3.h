#pragma once

#include <iostream>
#include <thread>


//Ϊ����֤����2�����⣬�Լ�����һ����
class Temp
{
public:
	mutable int m_i;
	//����ת�����캯������һ��intת��Ϊһ����
	Temp(int i) :m_i(i) { std::cout << "[Temp::Temp(int i)���캯��ִ��]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl; }
	Temp(const Temp& other) :m_i(other.m_i) { std::cout << "[Temp::Temp(const Temp &other)�������캯��ִ��]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl; }
	~Temp() { std::cout << "[Temp::~Temp()��������ִ��]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl; }

	//��Ա����ָ�����̺߳���
	void thread_work(int num)
	{
		std::cout << "[���߳�thread_workִ��]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl;
	}

	//������󴴽��߳�(�º���)
	void operator()(int num)
	{
		std::cout << "[���߳�operator()ִ��]" << this << " ThreadID = " << std::this_thread::get_id() << std::endl;
	}
};

void Test(const Temp& temp)
{
	std::cout << &temp << std::endl;
	return;
}
//------����2����֤------//end

//1.1 Ҫ���������1---detach�������ڴ��������
//void MyPrint(const int &i, char *pmybuf)
	//��i��Ȼ�������͵Ĳ�������ʵ������ֵ���ݣ��������߳�detach�����̣߳����߳��е�i��Ȼ�ǰ�ȫ��
	//�����߳��е�ָ�����pmybufָ����ڴ�ռ�������߳��д������MyBufferָ����ڴ�ռ䣬����ָ����detach���߳�ʱ�����Ի�������
//����ʹ��detachʱ���Ƽ����������Ͳ�������Ҫ����ָ�����
void MyPrint(const int i, const std::string& pmybuf)
{
	std::cout << i << std::endl;
	std::cout << pmybuf << std::endl;
	return;
}

//ʹ��Thread2���鿴�������캯����ִ��ʱ��
void Thread2(const Temp& obj)
{
	obj.m_i = 100;
	std::cout << "�߳�2����������ַΪ" << &obj << " ";
	std::cout << "�߳�2�߳�idΪ" << std::this_thread::get_id() << std::endl;
}

//��������ָ��
void Thread3(std::unique_ptr<int> ptr)
{
	std::cout << "�߳�3����������ַΪ" << ptr << " ";
	std::cout << "�߳�3�߳�idΪ" << std::this_thread::get_id() << std::endl;
}

int main()
{
	//���̺߳������Σ���һ���������캯����һ������Ϊ�̺߳�������������Ϊ�̺߳�������

	//һ��������ʱ������Ϊ�̲߳���
	// detach��������������
	//1.1 Ҫ���������1---�ڴ��������
	//1.2 Ҫ���������2---���߳����̺߳�������ʱ��ʽת��(�����ʽ����)������ʱ��
		//����Σ�գ�const char���͵�����MyBuffer�ڱ����ݸ�mytobj�߳�ʱ��Ҫ����const char��const string����ʽת�������ǿ�����ת��ǰ��MyBuffer����Ϊ���߳�ִ����϶������գ���ôʵ��ת�����ڴ�������Ѿ������յ��ڴ�
		//ֻҪ����ʱ������������Ϊ�������ݸ��̣߳���ô���������߳�ִ�����ǰ����������ݸ����̣߳��ǰ�ȫ��
	//1.3 �ܽ�
		//a.������ݼ����Ͳ���������ʹ��ֵ����
		//b.������������
			//i.��Ҫ������ʽ����ת��������ʵ�ʵĶ��󣬻����ں��������й�����ʱ����
			//ii.����������Ҫ�����������͵ı�����������ܻ���๹��һ��
		//���ۣ���ʹ��detach��ֻ����join�������Ͳ����ھֲ�����ʧЧ�����̶߳��ڴ�Ƿ����õ�����

	//������һ��̽����ʱ������Ϊ��������������
	//2.1 �߳�id�ĸ��ÿ���߳�ʵ���϶���Ӧ��һ�����֣���ͬ���߳����߳�id��ͬ
		//�߳�id����ͨ��C++��׼����ĺ�������ȡ: std::this_thread::get_id();
	//2.2 ��ʱ������ʱ��ץ��

	//�����������������ָ����Ϊ�̲߳���
	//3.1 ���������������̣߳���ʹ�������õ�ʵ���ϴ��ݵ�Ҳ�����߳��ڶ����һ�����������������߳��ڶԳ�Ա���иı䲻��Ӱ�쵽���߳��еĶ���
	//3.2 �����ܹ������߳��ж����̲߳���Ӱ��ı�����ʹ��std::ref(<����>...)
		//a.���߳��д�����������std::ref�����������̺߳���������������Ϊconst
		//b.�����߳��д���std::ref������������
	//3.3 ��������ָ��
		//a.uniqueָ����Ҫͨ��std::move(<ptr>)����

	//�ġ��ó�Ա����ָ�����̺߳���


	int MyVar = 1;
	int& MyVary = MyVar;
	char MyBuffer[] = "this is a buffer";

	//1.1 Ҫ���������2
	//std::thread mytobj(MyPrint, MyVar, MyBuffer);
	//std::thread mytobj(MyPrint, MyVar, std::string(MyBuffer));//�������2---����ǰʹ��MyBuffer����һ����ʱ������Ϊ�������ݸ����߳�


	//begin------����2����֤------
	//Ϊ�˸�ֱ�۵ؿ�����2���ڵĵ��������ʹ���Լ��������������֤
	//std::thread TestThread(Test, MyVar);//�ڴ�ӡHello֮��ִ�й��캯�������ڷ��գ�
	//std::thread TestThread(Test, Temp(MyVar));//�ڴ�ӡHello֮ǰִ��������ת�����캯��(������ʱ����)���������캯��(ʵ�ʴ���)����������(����ִ����Ϻ������ʱ����)����ȫ���������
	//TestThread.detach();
	//------����2����֤------end

	//mytobj.join();
	//mytobj.detach();

	////begin------2.2 ��ʱ������ʱ��ץ��------
	//std::cout << "���߳��߳�idΪ" << std::this_thread::get_id() << std::endl;
	//std::cout << "------ʹ����ʽ����ת��------" << std::endl;
	//std::thread thread2A(Thread2, MyVar);
	////ʹ����ʽ����ת��ʱ�������������߳��н��й���ģ�ʹ��detachʱ�ͻ��������
	//std::cout << "------ʹ����ʱ����------" << std::endl;
	//std::thread thread2B(Thread2, Temp(MyVar));
	////ʹ����ʱ����ʱ�����������߳��й���һ����ʱ����(����ת�����캯��)��Ȼ���������߳��е��ÿ������캯��(�����߳��н��еĴ���)�����������ʱ���󲢷��߳�ִ�к�����������ôdetach�������߳��е��þͲ����а�ȫ������

	//thread2A.join();
	//thread2B.join();
	////------end

	//begin------3.1-3.2�����߳��д��ݿɱ����------
	Temp obj(10);
	std::thread thread3a(Thread2, obj);
	thread3a.join();
	std::cout << obj.m_i << std::endl;//��ӡ10
	std::thread thread3b(Thread2, std::ref(obj));
	thread3b.join();
	std::cout << obj.m_i << std::endl;//��ӡ100
	//------end

	//begin------3.3��������ָ��------
	std::unique_ptr<int> ptr(new int(100));
	std::thread thread3(Thread3, std::move(ptr));//����std::move����
	thread3.join();//���������ָ��ָ���ַ�������̣߳�ʹ��detach�ֿ��ܻ�������
	//------end

	//begin------ʹ�����Ա����ָ�����̺߳���------
	Temp obj(10);
	std::thread Thread4A(&Temp::thread_work, obj, 15);//��Ҫ�����Ա����ָ�롢����֮����Ǵ��뺯���Ĳ���
	//����Ķ���������߳���ִ��һ�ο������죬���Կ���ʹ��detach
	Thread4A.detach();
	std::thread Thread4B(&Temp::thread_work, std::ref(obj), 15);//ͬ���ʹ��std::ref���������ý������̣߳�������ʱ�Ͳ���ʹ��detach��
	Thread4B.join();
	//------end

	std::cout << "Hello" << std::endl;
	return 0;
}
