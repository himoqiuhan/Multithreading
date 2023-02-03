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

			std::unique_lock<std::mutex> UniqueLock1(my_mutex1, std::try_to_lock);
			if (UniqueLock1.owns_lock())
			{
				//�õ�����
				msgRecvQueue.push_back(i);
			}
			else
			{
				std::cout << "---inMsgRecvQueueû�õ���---" << i <<std::endl;
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
		std::this_thread::sleep_for(duration);//�߳���Ϣһ��ʱ��

		if (!msgRecvQueue.empty())
		{
			command = msgRecvQueue.front();
			msgRecvQueue.pop_front();
			return true;
		}
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


	//һ��unique_lockȡ��lock_guard
	//unique_lock�Ǹ���ģ�壬�����У�һ���Ƽ�ʹ��lock_guard(lock_guardȡ��mutex.lock��mutex.unlock)
	//unique_lock��lock_guard������Ч�ʸ����ڴ桢ռ�ø���

	//����unique_lock���캯���ĵڶ�����(����Ե�����)
	//2.1 std::adopt_lock
	//�����û������Ѿ���lock�ˣ����ڹ��캯���в�����Ҫlock
	//ǰ�����ڹ���ǰ��������lock
	//2.2 std::try_to_lock
	//������mutex.lock()ȥ������mutex�������û��lock�ɹ����ڲ���lockҲ�᷵�أ��߳��еĳ������ִ�У�����������������
	//ǰ����ʹ��try_to_lockǰ�����ֶ�lock
	//2.3 std::defer_lock
	//��ʼ��һ��û��lock��uniqu_lock---ʹ��unique_lock����mutex����û��lock
	//ǰ����ʹ��defer_lockǰ�����ֶ�lock

	//����unique_lock�ĳ�Ա����
	//3.1 lock()
	//����
		//ע���Ƕ�unique_lock���������ã�������mutex
		/*
		std::unique_lock<std::mutex> UniqueLock(my_mutex,std::defer_lock);
		UniqueLock.lock();
		*/
	//3.2 unlock()
	//����
		//ע���Ƕ�unique_lock���������ã�������mutex
	//���߳�ִ���У������м���һ�ηǹ������ݴ���Ҫ��������ϣ���������ס��һ�飬�Ϳ��Ե���unlock��lock��������������
		/*
		std::unique_lock<std::mutex> UniqueLock(my_mutex,std::defer_lock);
		UniqueLock.lock();
		//...����������...
		UniqueLock.Unlock();
		//...����ǹ�������...
		UniqueLock.lock();
		//...�ٴδ���������...
		//...
		*/
	//ע��unique_lock������������ִ��unlockǰ���ж�mutex�Ƿ�lock������Ѿ��ֶ���ǰ�����������������оͲ�����ִ��unlock
	//3.3 try_lock()
	//���Լ��������ز���ֵ�������ɹ�����true��ʧ�ܷ���false
	//������try_to_lock���������ã�lockʧ��Ҳ��������
	//3.4 release()
	//�������������mutex����ָ�룬���ͷ�����Ȩ��Ҳ����˵�������mutex��unique_lock�������й�ϵ
	//ʹ��std::mutex*�������������պ�������ֵ
	//unlock()��release()������
		//unlock()�ǽ�������Ӧ��mutex��������unique_lock������;
		//release()���ͷţ�����ԭʼ��mutexָ�룬���ڲ���mutex��unique_lock�����ڵĳ�Ա���룬��mutex����ԭ״̬(ԭ����lock����lock��ԭ����unlock����unlock)
	//3.5 �ܽ�
	//a.Ϊʲô��Ҫ��unlock():��Ϊlock�����Ĵ����Խ�٣�ִ��Խ�죬��������Ч��Խ��
	//b.����Ҳ������ס�Ĵ���Ķ��ٳ�Ϊ�������ȣ�����һ���ô�ϸ������
		//��ס�Ĵ����٣�����ϸ��ִ��Ч�ʸߣ���ס�Ĵ���࣬���ȴ֣�ִ��Ч�ʵ�
		//Ҫѡ����ʵ����ȶԴ�����б���

	//�ġ�unique_lock����Ȩ�Ĵ���(mutex)
	//UniqueLock���Խ��Լ���my_mutex������Ȩת�Ƹ�����unique_lock����
	//ע�⣺unique_lock����Ȩ����ת�ƣ������ܸ���
	//1.1 ����1--ʹ��std::move()
	/*
	std::unique_lock<std::mutex> UniqueLock(my_mutex);//��ʱUniqueLock��my_mutex��������Ȩ
	std:unique_lock<std::mutex> NewUniqueLock(std::move(UniqueLock));
	*/
	//1.2 ����2--����return std::unique_lock<std::mutex>
	/*
	���磺
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
			std::unique_lock<std::mutex> UniqueLock = rtn_unique_lock();//�����unique_lock���ƶ����캯��������ʱ����ʵ����������Ȩת��
		}
	};
	*/

	return 0;
}