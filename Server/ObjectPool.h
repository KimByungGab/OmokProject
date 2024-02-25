#pragma once

#include <stack>
#include <mutex>

using namespace std;

template <typename T>
class ObjectPool
{
public:
	ObjectPool(int size = 100)
	{
		mMaxSize = size;

		for (int i = 0; i < mMaxSize; i++)
		{
			T* newObject = new T();
			mObjects.push(newObject);
		}
	}

	~ObjectPool()
	{
		lock_guard<recursive_mutex> guard(mLock);

		while (mObjects.empty() == false)
		{
			T* object = mObjects.top();
			mObjects.pop();
			
			delete object;
		}

		mMaxSize = 0;
	}

	T* PopObject()
	{
		lock_guard<recursive_mutex> guard(mLock);

		if (mObjects.empty())
			Expand();

		T* retVal = mObjects.top();
		mObjects.pop();

		return retVal;
	}

	void Expand()
	{
		lock_guard<recursive_mutex> guard(mLock);

		for (int i = 0; i < mMaxSize; i++)
		{
			T* newObject = new T();
			mObjects.push(newObject);
		}

		mMaxSize += mMaxSize;
	}

	void ReturnObject(T* object)
	{
		lock_guard<recursive_mutex> guard(mLock);
		mObjects.push(object);
	}

private:
	recursive_mutex mLock;
	stack<T*> mObjects;
	int mMaxSize = 0;
};

