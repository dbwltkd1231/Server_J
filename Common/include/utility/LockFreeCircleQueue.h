#pragma once

#include<iostream>
#include<atomic>

namespace Utility
{
	template<typename T>
	class LockFreeCircleQueue
	{
	private:
		int mQueueMaxSize;// ���������δ� queueMaxSize-1��ŭ �������ִ�. ����ť�� �ϳ��� ������ ��������ν� full�� empty���¸� �����ϱ� ����.
		std::atomic<int> mInputIndex;
		std::atomic<int> mOutputIndex;
		T* mBuffer;
	public:
		LockFreeCircleQueue<T>()
		{
			mInputIndex = 0;
			mOutputIndex = 0;
		}

		~LockFreeCircleQueue<T>()
		{
			delete[] mBuffer;
		}

		void Construct(int queueMaxSize)
		{
			this->mQueueMaxSize = queueMaxSize;
			mBuffer = new T[queueMaxSize];
		}

		//���⼭ &&�� rvalue reference�� �ǹ��Ѵ�.
		// rvalue reference�� �ӽ� ��ü�� �����ϴµ� ���Ǹ�, std::move()�� �Բ� ����Ͽ� ��ü�� �������� �̵��� �������� �Ҷ� ���ȴ�.
		//����ȭ �̵��� ���� �߿��� �������Ѵ�.
		bool push(T&& data)
		{
			int currentInputIndex = mInputIndex.load(std::memory_order_acquire);
			int nextIndex = (currentInputIndex + 1) % mQueueMaxSize;

			if (nextIndex == mOutputIndex.load(std::memory_order_acquire))
			{
				std::cout << "Queue is full" << std::endl;
				return false;
			}

			mBuffer[currentInputIndex] = std::move(data);
			mInputIndex.store(nextIndex, std::memory_order_release);// ������Ʈ �� release
			return true;
		}

		T pop()
		{
			int currentOutputIndex = mOutputIndex.load(std::memory_order_acquire); // �ֽ� outputIndex�� �����ͼ� ���Ѵ�.

			if (currentOutputIndex == mInputIndex.load(std::memory_order_acquire))//�����͸� �д� load �۾����� ����Ͽ� ���� ����ȭ.
			{
				std::cout << "Queue is empty" << std::endl;
				return T();
			}

			T data = std::move(mBuffer[currentOutputIndex]);
			mOutputIndex.store((currentOutputIndex + 1) % mQueueMaxSize, std::memory_order_release);// �����͸� ������ ����� �� �ε����� ������Ʈ�� �� ���
			return data;
		}

		bool empty()
		{
			return mInputIndex.load(std::memory_order_acquire) == mOutputIndex.load(std::memory_order_acquire);
		}

		int size()
		{
			auto input = mInputIndex.load(std::memory_order_acquire);
			auto output = mOutputIndex.load(std::memory_order_acquire);

			if (input >= output)
				return input - output;
			else
				return mQueueMaxSize - mOutputIndex + input;
		}

		int capacity()
		{
			return mQueueMaxSize;
		}

		void clear()
		{
			mInputIndex.store(0, std::memory_order_release);
			mOutputIndex.store(0, std::memory_order_release);
		}

		void print()
		{
			std::cout << "Input Index: " << mInputIndex << ", Output Index: " << mOutputIndex << std::endl;
			for (int i = 0; i < size(); i++)
			{
				std::cout << mBuffer[(mOutputIndex + i) % mQueueMaxSize] << " ";
			}
			std::cout << std::endl;
		}

		T Front()
		{
			return mBuffer[mOutputIndex.load(std::memory_order_acquire)];
		}
	};
}
