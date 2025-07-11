#pragma once

#include<iostream>
#include<atomic>

namespace Utility
{
	template<typename T>
	class LockFreeCircleQueue
	{
	private:
		int mQueueMaxSize;// 실질적으로는 queueMaxSize-1만큼 넣을수있다. 원형큐는 하나의 공간을 비워둠으로써 full과 empty상태를 구분하기 때문.
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

		//여기서 &&은 rvalue reference를 의미한다.
		// rvalue reference는 임시 객체를 참조하는데 사용되며, std::move()와 함께 사용하여 객체의 소유권을 이동을 목적으로 할때 사용된다.
		//최적화 이동을 위해 중요한 역할을한다.
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
			mInputIndex.store(nextIndex, std::memory_order_release);// 업데이트 후 release
			return true;
		}

		T pop()
		{
			int currentOutputIndex = mOutputIndex.load(std::memory_order_acquire); // 최신 outputIndex를 가져와서 비교한다.

			if (currentOutputIndex == mInputIndex.load(std::memory_order_acquire))//데이터를 읽는 load 작업에서 사용하여 성능 최적화.
			{
				std::cout << "Queue is empty" << std::endl;
				return T();
			}

			T data = std::move(mBuffer[currentOutputIndex]);
			mOutputIndex.store((currentOutputIndex + 1) % mQueueMaxSize, std::memory_order_release);// 데이터를 완전히 기록한 뒤 인덱스를 업데이트할 때 사용
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

/*

shard_ptr타입을 T로 사용시 객체가 일부 유실되는 문제가 발생함.

LockFreeCircleQueue가 shared_ptr<T> 타입을 저장할 때, 내부에서 new T[queueMaxSize]로 배열을 생성하는 구조이기 때문에…
- shared_ptr<EventWorker> 배열이 전부 nullptr로 초기화됨
- 이후 push(std::move(shared_ptr))을 해도, 복사/이동이 제대로 되지 않거나
- pop()으로 꺼낸 객체가 nullptr이어서 사용 중 크래시 발생


- 큐를 EventWorker 값 타입으로 변경 (즉, LockFreeCircleQueue<EventWorker>)
- → new T[]로 생성된 객체들이 올바르게 구성되며
- → move와 copy가 값 타입으로 확실히 작동함
- → 객체 수명, 초기화, 안정성 문제 해결


*/