/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef QUEUE_H
#define QUEUE_H

#include <engine/common/common.h>

// Queue of either POD or pointers
template <class T, sint16 maxElements>
class Queue
{
public:
	Queue<T, maxElements>()
	{
		m_count = 0;
		m_headLocation = 0;
		m_tailLocation = 0;
		m_maxElements = maxElements;
	}

	~Queue<T, maxElements>()
	{
		Clear();
	}

	// Is the queue empty
	bool IsEmpty()
	{
		return m_count == 0;
	}
	bool IsFull()
	{
		return m_count == m_maxElements;
	}

	// Traditional add-to-back
	void push(T data)
	{
		// Assert the size
		if (m_count == m_maxElements)
		{
			ASSERT_RETURN(0 && "Queue is full.");
		}

		// Increment the tail pointer
		m_tailLocation = (m_count > 0) ? ((m_tailLocation + 1) % m_maxElements) : (m_headLocation);
		
		// Add the data
		m_elements[m_tailLocation] = data;

		// Increment the size
		m_count++;
	}

	// Unconventional add-to-front
	void push_front(T data)
	{
		// Assert the size
		if (m_count == m_maxElements)
			ASSERT_RETURN(0 && "Queue is full.");

		// Pull the head pointer forward
		if (m_count > 0)
			m_headLocation = (m_headLocation + m_maxElements - 1) % m_maxElements;

		// Add the data
		m_elements[m_headLocation] = data;

		// Increment the size
		m_count++;
	}

	// Unconventional remove-from-back
	T pop_back()
	{
		// Check the size
		if (m_count == 0)
			ASSERT_RETURN_VALUE(0 && "Queue is empty.", m_elements[0]);
		
		// Retrieve the data
		T temp = m_elements[m_tailLocation];

		// Subtract the count
		m_count--;
		
		// Update the tail pointer
		if (m_count > 0)
			m_tailLocation = (m_tailLocation + m_maxElements - 1) % m_maxElements;

		return temp;
	}

	// Conventional remove-from-front
	T pop()
	{
		// Check the size
		if (m_count == 0)
			return m_elements[0];

		// Retrieve the data
		T temp = m_elements[m_headLocation];

		// Subtract the count
		m_count--;

		// Update the head pointer
		if (m_count > 0)
			m_headLocation = (m_headLocation + 1) % m_maxElements;

		return temp;
	}

	// Peek at the first element without popping it
	T peek()
	{
		// Check the size
		if (m_count == 0)
			return m_elements[0];
		return m_elements[m_headLocation];
	}

	// Get an element somewhere in the middle
	T& operator[](int index)
	{
		if (index >= m_count || index < 0)
			ASSERT_RETURN_VALUE(0 && "index was out of range", m_elements[0]);

		int trueIndex = (index + m_headLocation) % m_maxElements;
		return m_elements[trueIndex];
	}

	// Return the number of elements
	sint16 Count() const
	{
		return m_count;
	}

	// Clear the queue
	void Clear()
	{
		m_headLocation = 0;
		m_tailLocation = 0;
		m_count = 0;
	}



	struct Iterator
	{
		T* elems;
		sint16 location;
		sint16 count;
		sint16 numElements;

		Iterator& operator++(int)
		{
			location = (location + 1) % numElements;
			if (count > 0)
				count--;
			else
				location = 0;
			return *this;
		}
		Iterator& operator++()
		{
			location = (location + 1) % numElements;
			if (count > 0)
				count--;
			else
				location = 0;
			return *this;
		}
		Iterator& operator--()
		{
			location = (location + maxElements - 1) % numElements;
			count++;
			return *this;
		}
		Iterator& operator--(int)
		{
			location = (location + maxElements - 1) % numElements;
			count++;
			return *this;
		}
		bool operator!=(Iterator& rhs)
		{
			if (count == rhs.count)
				return false;
			return location != rhs.location;
		}
		bool operator==(Iterator& rhs)
		{
			if (count != rhs.count)
				return false;
			return location == rhs.location;
		}

		T& operator*()
		{
			return elems[location];
		}
		bool IsEnd()
		{
			return count == 0;
		}
	};

	// Iterator
	Iterator Begin()
	{
		Iterator it;
		it.numElements = m_maxElements;
		it.location = m_headLocation;
		it.count = m_count;
		it.elems = m_elements;
		return it;
	}
	Iterator End()
	{
		Iterator it;
		it.location = (m_tailLocation + 1) % m_maxElements;
		it.numElements = m_maxElements;
		it.count = 0;
		it.elems = m_elements;
		return it;
	}

	// Erase an element from the queue
	Iterator Erase(Iterator it)
	{
		if (it == End())
			return it;

		for (sint16 i = it.location; i != (it.location + it.count - 1); i++)
		{
			m_elements[i%m_maxElements] = m_elements[(i + 1) % m_maxElements];
		}
		m_count--;

		it.count--;
		return it;
	}
private:
	T m_elements[maxElements];
	sint16 m_maxElements;
	sint16 m_headLocation;
	sint16 m_tailLocation;
	sint16 m_count;
};

#endif
