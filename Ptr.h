#pragma once

namespace Ptr
{
	//pointer that deallocates heap memory once it has exited the scope
	//for safety, there can only be 1 pointer to each heap allocated block of memory
	template <typename T>
	class ScopedPtr
	{
	public:
		//defualt constructor
		ScopedPtr();
		//constructor that takes in a pointer (ScopedPtr<T> ptr(new T);)
		explicit ScopedPtr(T* ptr);

		//deleted functions to avoid copying of pointers (use RefPtr)
		ScopedPtr(const ScopedPtr&) = delete;
		ScopedPtr& operator=(const ScopedPtr&) = delete;

		//rvalue constructor and move assignment operator (ScopedPtr<T> ptr; ptr = ScopedPtr<T>(new T);)
		//std::move also supported (ScopedPtr<T> ptr, ptr2; ptr = std::move(ptr2)) <- calls move assignment operator
		ScopedPtr(ScopedPtr&& other) noexcept;
		ScopedPtr& operator=(ScopedPtr&& other) noexcept;

		//destructor
		~ScopedPtr();

		//functions that return the raw pointer
		T* Get() const;
		T* operator->() const;

		//functions that dereferences pointer
		T& Dereference() const;
		T& operator*() const;

	private:
		//function for cleanup
		void Clean();

	private:
		T* ptr;
	};

	//the only difference between the reference pointer and the scoped pointer is that reference pointers
	//allow multiple pointers to the same memory address
	//keeps a count of the amount of pointers, and the memory gets deallocated once the count reaches 0
	template <typename T>
	class RefPtr
	{
	public:
		//default constructor
		RefPtr();
		//constructor that takes in a pointer (RefPtr<T> ptr(new T);)
		explicit RefPtr(T* ptr);

		//copy constructor and copy assignment operator (RefPtr<T> ptr(new T); RefPtr<T> ptr2 = ptr)
		RefPtr(const RefPtr& other);
		RefPtr& operator=(const RefPtr& other);

		//rvalue constructor and move assignment operator (RefPtr<T> ptr; ptr = RefPtr<T>(new T);)
		//std::move also supported (RefPtr<T> ptr, ptr2; ptr = std::move(ptr2)) <- calls move assignment operator
		RefPtr(RefPtr&& other) noexcept;
		RefPtr& operator=(RefPtr&& other) noexcept;

		//destructor
		~RefPtr();

		//functions that return the raw pointer
		T* Get() const;
		T* operator->() const;

		//functions that dereferences pointer
		T& Dereference() const;
		T& operator*() const;

		//returns the amount of pointers to a memory address
		const size_t GetRefCount() const;

	private:
		//function to increase and decrease the reference count
		void IncRef();
		void DecRef();

		//function for cleanup
		void Clean();

	private:
		T* ptr;
		size_t* refs;
	};

	//calls constructor for an object (ScopedPtr<T> ptr = InitScopedPtr<T>(parameters);)
	//for general safety so that memory is allocated here and not in your program
	//does the same thing as calling the explicit constructor
	template <typename T, typename ... Args>
	ScopedPtr<T> InitScopedPtr(Args&& ... mArgs)
	{
		return ScopedPtr<T>(new T(std::forward<Args>(mArgs)...));
	}

	//calls constructor for an object (RefPtr<T> ptr = InitRefPtr<T>(parameters);)
	//for general safety so that memory is allocated here and not in your program
	//does the same thing as calling the explicit constructor
	template <typename T, typename ... Args>
	RefPtr<T> InitRefPtr(Args&& ... mArgs)
	{
		return RefPtr<T>(new T(std::forward<Args>(mArgs)...));
	}

	template <typename T>
	ScopedPtr<T>::ScopedPtr()
		: ptr(nullptr)
	{
	}

	template <typename T>
	ScopedPtr<T>::ScopedPtr(T* ptr) 
		: ptr(ptr)
	{
	}

	template <typename T>
	ScopedPtr<T>::ScopedPtr(ScopedPtr&& other) noexcept
		//copy the other pointer
		: ptr(other.ptr)
	{
		//set the other pointer to point to nothing
		other.ptr = nullptr;
	}

	template <typename T>
	ScopedPtr<T>& ScopedPtr<T>::operator=(ScopedPtr&& other) noexcept
	{
		//if they are not the same thing
		if (this != &other)
		{
			//unallocate any memory that could be allocated
			Clean();

			//copy the other pointers data and set the other pointer to point to nothing
			ptr = other.ptr;
			other.ptr = nullptr;
		}

		return *this;
	}

	template <typename T>
	ScopedPtr<T>::~ScopedPtr()
	{
		Clean();
	}

	template <typename T>
	T* ScopedPtr<T>::Get() const
	{
		return ptr;
	}

	template <typename T>
	T* ScopedPtr<T>::operator->() const
	{
		return ptr;
	}

	template <typename T>
	T& ScopedPtr<T>::Dereference() const
	{
		return *ptr;
	}

	template <typename T>
	T& ScopedPtr<T>::operator*() const
	{
		return *ptr;
	}

	template <typename T>
	void ScopedPtr<T>::Clean()
	{
		//if the pointer is not pointing to nothing, unallocate the memory
		if (ptr != nullptr)
			delete ptr;
	}

	template <typename T>
	RefPtr<T>::RefPtr()
		//by default, ref count is 0 since memory has not been allocated
		: ptr(nullptr), refs(new size_t(0))
	{
	}

	template <typename T>
	RefPtr<T>::RefPtr(T* ptr)
		//set the reference count to start at 1
		: ptr(ptr), refs(new size_t(1))
	{
	}

	template <typename T>
	RefPtr<T>::RefPtr(const RefPtr& other)
		//copy the other pointers data
		: ptr(other.ptr), refs(other.refs)
	{
		//increase the reference count since we have a new pointer
		IncRef();
	}

	template <typename T>
	RefPtr<T>& RefPtr<T>::operator=(const RefPtr& other)
	{
		//if they are not the same thing
		if (this != &other)
		{
			//unallocate any memory that could be allocated/decrease the reference count 
			Clean();

			//copy the other pointers data
			ptr = other.ptr;
			refs = other.refs;

			//increase the reference count
			IncRef();
		}
		//it is important to keep in mind, that we are tracking our reference count via a pointer.
		//this means by copying the data, I am changing the memory address the reference count is pointing to,
		//so decreasing then increasing the reference count makes sense, as I am doing it for two different smart pointers

		return *this;
	}

	template <typename T>
	RefPtr<T>::RefPtr(RefPtr&& other) noexcept
		//copy the other pointers data
		: ptr(other.ptr), refs(other.refs)
	{
		//set the others data to point to nothing
		other.ptr = nullptr;
		other.refs = nullptr;

		//here we do not increase the reference count
		//because we are taking in an rvalue, a temporary and we are essentially stealing the data
		//it makes no sense to increase the reference count, because we are just moving pre-exisiting data into a new container
	}

	template <typename T>
	RefPtr<T>& RefPtr<T>::operator=(RefPtr&& other) noexcept
	{
		//if they are not the same thing
		if (this != &other)
		{
			//unallocate any memory that could be allocated/decrease the reference count 
			Clean();

			//copy the other pointers data
			ptr = other.ptr;
			refs = other.refs;

			//set the other data to point to nothing
			other.ptr = nullptr;
			other.refs = nullptr;
		}
		
		return *this;
	}

	template <typename T>
	RefPtr<T>::~RefPtr()
	{
		Clean();
	}

	template <typename T>
	T* RefPtr<T>::Get() const
	{
		return ptr;
	}

	template <typename T>
	T* RefPtr<T>::operator->() const
	{
		return ptr;
	}

	template <typename T>
	T& RefPtr<T>::Dereference() const
	{
		return *ptr;
	}

	template <typename T>
	T& RefPtr<T>::operator*() const
	{
		return *ptr;
	}

	template <typename T>
	const size_t RefPtr<T>::GetRefCount() const
	{
		return *refs;
	}

	template <typename T>
	void RefPtr<T>::IncRef()
	{
		//if the memory has been allocated then increase the count
		if (ptr == nullptr)
			return;

		(*refs)++;
	}

	template <typename T>
	void RefPtr<T>::DecRef()
	{
		//if the memory has been allocated then decrease the count
		if (ptr == nullptr)
			return;

		(*refs)--;
	}

	template <typename T>
	void RefPtr<T>::Clean()
	{
		//decrease the reference count
		DecRef();

		//if it reaches 0, only then does the memory get freed
		if (*refs == 0)
		{
			if (ptr != nullptr)
				delete ptr;

			delete refs;
		}
	}
}
