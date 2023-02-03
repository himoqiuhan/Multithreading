#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <list>

#define Use_call_once true

#if Use_call_once
std::once_flag g_flag;//���Ǹ�ϵͳ����ı�ǣ�ͬ����Ҫ��ȫ�ֱ���
#else
std::mutex resource_mutex;//����ȫ��mutex���������̷߳��ʵ��Ļ�������ͬһ��
#endif
class Singleton
{
private:
	Singleton(){}//˽�л����캯��
	~Singleton() { std::cout << "Destructor!" << std::endl; }

	static void CreateInstance()//ֻ����һ�εĺ���
	{
		m_instance = new Singleton();
		static Reclaim rec;
	}

private:
	static Singleton* m_instance;//��̬��Աָ�룬ָ���Լ��ľ�̬ʵ��

public:
	Singleton(const Singleton& other) = delete;//���ǿ������캯����ȷ��Singleton��Ψһ��

	static Singleton* GetInstance()//�����ڲ�ʵ����ָ��(���߷�������)
	{
#if Use_call_once
		std::call_once(g_flag, CreateInstance);//�����߳�ͬʱִ�е�����ʱ������һ�߳�Ҫ������һ���߳�ִ�����CreateInstance�������ı�g_flag��Ǻ����ж�g_flag��������������Ϊ��ִ�У��򲻻���ִ��CreateInstance����
#else
		if (m_instance == nullptr)//˫�ؼ�飬���Ч�ʣ���ֹ������ع���unique_lock�˷�����
		//��Ҫʹ��lock������ԭ��
			//���m_instance != nullptr����������һ����ʾm_instance�Ѿ���new����
			//���m_instance == nullptr������������һ���ܱ�֤m_instance��û�б�new��������һ���߳���׼��ִ��m_instance = new Singleton();��һ�д��룬����һ���߳�ǡ�������if�������ж�
		{
			std::unique_lock<std::mutex> my_mutex(resource_mutex);//�Զ�����
			if (m_instance == nullptr)
			{
				m_instance = new Singleton();
				static Reclaim rec;//ͨ��һ��Ƕ�������ͷ�ʵ����ָ��
				//������������ֱ�ӵ���delete����Ϊ��ʼ���ն�û�д���ʵ���࣬���Բ������ⲿ�������������������Ҫ����һ��Ƕ���࣬ͨ��Ƕ���������������singleton�������������ͷ��ڴ�
			}
		}
#endif
		return m_instance;
	}

	//ʹ��Ƕ�������singleton���ڴ�
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
Singleton* Singleton::m_instance = nullptr;//��ʼ����̬��Աʵ��ָ��

void MyThread()
{
	std::cout << std::this_thread::get_id() << "�߳̿�ʼ��" << std::endl;
	Singleton* MySingleton = Singleton::GetInstance();//�˴��������
	std::cout << std::this_thread::get_id() << "�߳̽�����" << std::endl;
	return;
}

int main()
{
	//һ�����ģʽ����
	//�����ģʽ���������һЩ�ǳ���д����������ά�����㣬���Ǳ��˽ӹܡ��Ķ���������ѡ�ʹ�á����ģʽ������д�����Ĵ���ܻ�ɬ
	//���ģʽ��Դ�ڳ���ԱӦ���ر�����Ŀ��ʱ������Ŀ�Ŀ������顢ģ�黮�־����ܽ���������ģʽ(���еĿ������󣬺���е������ܽ�)

	//�����������ģʽ
	//�������ģʽʹ�õ�Ƶ�ʱȽϸ�
	//����(Singleton)��������Ŀ�У���ĳ������ĳЩ������࣬���ڸ���Ķ���ֻ�ܴ���һ��
	//�������д��
		//1.˽�л����캯��
		//2.ָ��һ����ʵ���ľ�̬��Ա����
		//3.Get��̬�������ظþ�ָ̬��
		//4.main������ͨ��Get������ȡ�þ�ָ̬�룬����ͨ�����ýӿھ�̬������ִ�к���

	//�����������ģʽ��������������������
	//����������ܹ������߳��У������߳�ǰ��ʼ�������ڲ�������
	//���ٵ����⣺��Ҫ�������Լ��������߳�(�����߳�)��������������Ķ����������߳̿���ʹ�����������
	//���ǿ��ܻ�����GetInstance()������Ҫ��������

	//��Ȼ����������ں�����ͬ���������������̣߳����Դ˴���������ͨ·ͬʱִ��MyThread()
	std::thread MyThread1(MyThread);
	std::thread MyThread2(MyThread);
	MyThread1.join();
	MyThread2.join();


	//�ġ�std::call_once()
	//C++11����ĺ������ú����ĵڶ���������һ��������method()��call_once�Ĺ����Ǳ�֤����method()��ʹ�ڶ��߳���Ҳֻ������һ��
	//call_once�߱��������������ұȻ���������Ч
	//call_once��Ҫ��һ����ǽ��ʹ�ã�std::once_flag
		//call_once����ͨ����������������Ӧ�ĺ���method()�Ƿ�ִ�У�����call_once()�ɹ���call_once()�Ͱ�once_flag����Ϊ�ѵ���״̬�������ٴε���call_once()��ֻҪonce_flag������Ϊ�ˡ��ѵ��á�״̬����ô��Ӧ�ĺ���method()�Ͳ����ٱ�ִ����
	


	std::cout << "hello" << std::endl;
	return 0;
}