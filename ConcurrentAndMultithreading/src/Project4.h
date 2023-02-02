#pragma once

#include <iostream>
#include <thread>
#include <mutex>

#include <vector>
#include <list>

//�ó�Ա������Ϊ�̺߳���
class A
{
public:
	//���յ�����Ϣ���뵽һ�����е��߳�
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue()ִ�У�����һ��Ԫ��" << i << std::endl;
			//my_mutex2.lock();//ʵ�ʹ�������������һ�����ţ�����������Ҫ������ͬ�����ݹ����
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
		//std::lock_guard<std::mutex> LockGuard(my_mutex);//lock_guard<mutex>��ֻ�п������캯��
		//����ʹ��lock_guard������ʹ��lock()��unlock()

		/*my_mutex1.lock();
		my_mutex2.lock();*/

		std::lock(my_mutex1, my_mutex2);

		std::lock_guard<std::mutex> LockGuard1(my_mutex1,std::adopt_lock);
		std::lock_guard<std::mutex> LockGuard2(my_mutex2,std::adopt_lock);
		
		//��Ϊ�·��Թ��������в��������Խ���lock()
		if (!msgRecvQueue.empty())
		{
			//��Ϣ���зǿ�
			command = msgRecvQueue.front();//���ص�һ��Ԫ��
			msgRecvQueue.pop_front();//�Ƴ���һ��Ԫ��
			//my_mutex.unlock();//��Ҫ���Ƿ�֧�ڵ�unlock()
			my_mutex1.unlock();
			my_mutex2.unlock();
			return true;
		}
		//my_mutex.unlock();//��Ҫ���Ƿ�֧�ڵ�unlock()
		my_mutex1.unlock();
		my_mutex2.unlock();
		return false;
	}

	//�����ݴ���Ϣ����ȡ�����߳�
	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; i++)
		{
			bool result = outMsgLULProc(command);
			if (result)
			{
				std::cout << "outMsgRecvQueue()ִ�У�ȡ��һ��Ԫ��" << command << std::endl;
			}
			else
			{
				std::cout << "outMsgRecvQueue()ִ�У�Ŀǰ��Ϣ������Ϊ��" << i << std::endl;
			}
		}
		std::cout << std::endl;
	}

private:
	std::list<int> msgRecvQueue;//ģ����Ϣ���У������������
	std::mutex my_mutex1;//������һ��������
	std::mutex my_mutex2;//������һ��������
};


int main()
{
	A obj;
	std::thread myOutMsgObj(&A::outMsgRecvQueue, std::ref(obj));//�������ã���֤�߳���ʹ�õ���ͬһ�����������
	std::thread myInMsgObj(&A::inMsgRecvQueue, std::ref(obj));//�������ã���֤�߳���ʹ�õ���ͬһ�����������
	myOutMsgObj.join();
	myInMsgObj.join();

	//�����������ݣ�ĳ���̲߳���ʱ�ô���ѹ���������ס������������������ݵ��̱߳���ȴ�����
	// 
	//һ��������(mutex)�Ļ�������
	//��������һ����������һ����������̳߳�����lock()��Ա������������ֻ��һ���߳��ܹ������ɹ�(�ɹ��ı�־Ϊlock()��������)�����û���ɹ�����ô���̻Ῠ��lock()��ִ���ϣ�ֱ�������ɹ���
	//ʹ�����̣���ʹ�ù�������ǰ������ʹ����ɺ����
	//��������ʹ��ҪС�ģ��������ݲ���Ҳ���٣����˴ﲻ������Ч��������Ӱ��Ч��

	//�������������÷�
	//��Ҫ����ͷ�ļ�mutex #include <mutex>
	//2.1 lock(),unlock()
		//���裺��lock(),�����������ݣ���unlock()
		//ʹ�ù���lock()��unlock()Ҫ�ɶ�ʹ�ã���lock()��ȻҪ��unlock()����lock()��unlock()�ĵ��ô����Գ� [n��lock()�����Ӧn��unlock()]
	//2.2 std::lock_guard��ģ��
	//��lock()��unlock()������ǳ����Ų飬����C++������һ����std::lock_guard����ģ��(�Զ�����unlock������������ָ��)
	//std::lock_guard��ֱ��ȡ��lock()��unlock():Ҳ����˵������std::lock_guard֮��Ͳ���ʹ��lock()��unlock()
		//std::lock_guard��ģ�幹��ʱ����lock()������ʱ����unlock()---��������--->ͨ��������ĵ���ʵ����ǰunlock
	//std::lock_guard��ȱ��:���޷���ݵؿ���unlockʱ���ڶ���һ���װ���˷�����

	//��������
	//3.1 ������ʾ
	//������������(���������������������ϵĻ�����������)����JinLock������YinLock�������߳�A��B��(1)�߳�Aִ�е�ʱ��A�����������ѽ���lock()�ɹ���Ȼ��������ȥlock������(2)��ʱ����ϵͳ���ȣ������������л����߳�Bִ���ˣ�B����������ǡ��������û�б�lock���߳�B���ܰ�����lock()�ɹ���Ȼ���߳�B�ٳ���ȥlock������(3)I.�߳�A�ò������������߲���ȥ�������JinLock.unlock()�޷�ִ�У����Խ����ⲻ��;II.�߳�B�ò��������������߲���ȥ�������YinLock.unlock()�޷�ִ��; ��Ҷ��ڵȴ�����ʱ�������Ͳ�����
	//3.2 ������һ��������
		//ֻҪ��֤������������������˳��һ�¾Ͳ�������
	//3.3 std::lock()����ģ��
	//���ã�һ������ס�������������ϵĻ�����(����������û���������ޣ�1������)�������ڴ�������������ʱ��ų���
	//��������Ϊ����߳�������˳���������ķ���
	//�������������һ��û��ס��std::lock()�ͻ�ȴ���ֱ�����л���������ס�˲Ż��������ִ��
		//���޷�ʵ��ȫ��������������ʱ��std::lock()�Ὣ���Լ������Ļ��������� �������� Ҫô������ȫ��ס��Ҫô������ȫû��
	//std::lock(my_mutex1, my_mutex2);
	//3.4 std::lock_guard��std::adopt_lock����
	//���ʹ��std::lock��ͳһ������std::lock_guard��ģ����Զ�����
	/*
	std::adopt_lock��������ģ���๹�쵫��lock(���캯��������)��������Ҫ�ڹ���std::lock_guard�����֮ǰʹ��std::lock()��ס
		std::lock(my_mutex1, my_mutex2);
		std::lock_guard<std::mutex> LockGuard1(my_mutex1,std::adopt_lock);
		std::lock_guard<std::mutex> LockGuard2(my_mutex2,std::adopt_lock);
	*/
	

	std::cout << " Hello " << std::endl;
	return 0;
}