/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK Basic Components FileReader.
 *
 * REDHAWK Basic Components FileReader is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK Basic Components FileReader is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

/*
 * Thread-safe queue template definition
 */
#ifndef THREADSAFE_QUEUE_HPP
#define THREADSAFE_QUEUE_HPP

/* boost::bind */
#include <boost/bind.hpp>
/* BOOST_CONCEPT_*, boost::Assignable, boost::DefaultConstructible */
#include <boost/concept_check.hpp>
/* boost::condition_variable */
#include <boost/thread/condition_variable.hpp>
/* boost::mutex */
#include <boost/thread/mutex.hpp>
/* std::deque */
#include <deque>

/* start of namespace */
namespace threadsafe {

/** Thread-safe queue template
 * \pre T must be Assignable
 */
template<typename T>
class Queue
{
	/** ensure T is Assignable */
	BOOST_CONCEPT_ASSERT((boost::Assignable<T>));
public:

	/** Constructor
	 * \param depth maximum queue depth (0 for unlimited)
	 */
	Queue(size_t depth=0): depth_(depth)
	{
	}

	/** Destructor */
	virtual ~Queue()
	{
	}

	/** Push an object into queue
	 * \param[in] obj object to push into queue
	 * \interruption boost::condition_variable::wait()
	 */
	void push(const T& obj)
	{
		exclusive_lock lock(mutex);
		not_full_.wait(lock, boost::bind(&Queue<T>::isNotFull_, this));
		push_(obj);
	}

	void pushFront(const T& obj)
	{
		exclusive_lock lock(mutex);
		not_full_.wait(lock, boost::bind(&Queue<T>::isNotFull_, this));
		push_front_(obj);
	}

	/** Push an object into queue (with timeout)
	 * \param[in] obj object to push into queue
	 * \param     ms  number of milliseconds until timeout (or 0 for no wait)
	 * \retval true  \c obj sucessfully pushed into queue
	 * \retval false \c obj could not be pushed into queue
	 * \interruption boost::condition_variable::timed_wait() if \c ms > 0
	 */
	bool tryPush(const T& obj, unsigned int ms=0)
	{
		bool result = false;
		exclusive_lock lock(mutex);
		/* choose test to determine if data can be popped */
		if (ms == 0)
		{
			/* try without waiting */
			result = isNotFull_();
		}
		else
		{
			/* wait for timeout (interruption point) */
			boost::system_time const timeout=boost::get_system_time() +
				boost::posix_time::milliseconds(ms);
			result = not_full_.timed_wait(lock, timeout,
				boost::bind(&Queue<T>::isNotFull_, this));
		}
		/* push if possible */
		if (result)
		{
			push_(obj);
		}
		return result;
	}

	/** Pop an object out of queue
	 * \returns Object from queue
	 * \interruption boost::condition_variable::wait()
	 */
	T pop()
	{
		exclusive_lock lock(mutex);
		not_empty_.wait(lock, boost::bind(&Queue<T>::isNotEmpty_, this));
		return pop_();
	}

	/** Try to pop an object off of queue (with optional timeout)
	 * \param[out] obj destination for object from queue
	 * \param      ms  number of milliseconds until timeout (or 0 for no wait)
	 * \retval true  \c obj sucessfully popped off of queue
	 * \retval false \c obj could not be popped off of queue (\c obj has not
	 * been modified)
	 * \interruption boost::condition_variable::timed_wait() if \c ms > 0
	 */
	bool tryPop(T& obj, unsigned int ms=0)
	{
		bool result = false;
		exclusive_lock lock(mutex);
		/* choose test to determine if data can be popped */
		if (ms == 0)
		{
			/* try without waiting */
			result = isNotEmpty_();
		}
		else
		{
			/* wait for timeout (interruption point) */
			boost::system_time const timeout=boost::get_system_time() +
				boost::posix_time::milliseconds(ms);
			result = not_empty_.timed_wait(lock, timeout,
				boost::bind(&Queue<T>::isNotEmpty_, this));
		}
		/* pop if possible */
		if (result)
		{
			obj = pop_();
		}
		return result;
	}

	/** Set a new maximum queue depth
	 * \param depth maximum queue depth (0 for unlimited)
	 * \returns Previous value of depth
	 */
	size_t setDepth(size_t depth)
	{
		exclusive_lock lock(mutex);
		size_t previous = depth_;
		/* if increasing depth (or changing from limited to unlimited) */
		if (previous && (!depth || (depth > previous)))
		{
			not_full_.notify_all();
		}
		depth_ = depth;
		return previous;
	}

	/** Get maximum queue depth
	 * \returns Maximum queue depth (0 for unlimited)
	 */
	size_t getDepth() const
	{
		size_t depth;
		exclusive_lock lock(mutex);
		depth = depth_;
		return depth;
	}

	/** Get queue usage
	 * \returns Number of elements in queue
	 * \note Value is for diagnostic purposes only. Acting upon this value
	 * would be a non-atomic operation (and thus not thread-safe).
	 */
	size_t getUsage() const
	{
		exclusive_lock lock(mutex);
		return container_.size();
	}

protected:
private:
	/** Mutex storage */
	mutable boost::mutex mutex;
	/** Exclusive lock type (scoped) */
	typedef boost::mutex::scoped_lock exclusive_lock;
	/** Not empty condition */
	boost::condition_variable not_empty_;
	/** Not full condition */
	boost::condition_variable not_full_;
	/** Internal queue container */
	std::deque<T> container_;
	/** Maximum queue depth (or 0 for unlimited) */
	size_t depth_;

	/** Test if queue is not empty
	 * \retval true  queue is not empty
	 * \retval false queue is empty
	 */
	bool isNotEmpty_()
	{
		return !container_.empty();
	}

	/** Test if queue is not full
	 * \retval true  queue is not full
	 * \retval false queue is full
	 */
	bool isNotFull_()
	{
		return (depth_ == 0) || (container_.size() < depth_);
	}

	/** Internal pop from queue
	 * \pre queue is not empty
	 * \returns Object from queue
	 */
	T pop_()
	{
		T value = container_.front();
		container_.pop_front();
		not_full_.notify_one();
		return value;
	}

	/** Internal push into queue
	 * \pre queue is not full
	 * \param obj object to push into queue
	 */
	void push_(const T& obj)
	{
		container_.push_back(obj);
		not_empty_.notify_one();
	}


	void push_front_(const T& obj)
	{
		container_.push_front(obj);
		not_empty_.notify_one();
	}
};

/* end of namespace */
}

#endif
