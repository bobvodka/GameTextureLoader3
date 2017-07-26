// GTL Memory iterators for moving over image data
#ifndef GTL_MEMORYITERATORS_HPP
#define GTL_MEMORYITERATORS_HPP
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace GTLMemoryIterators
{
	template<class T>
	class forward_memory_iter : 
		public boost::iterator_facade<forward_memory_iter<T>,typename T, boost::forward_traversal_tag >
	{
	public:
		struct enabler {};

		forward_memory_iter() : T(0)
		{}
		explicit forward_memory_iter(T* p) : data(p)
		{}
		template<class OtherValue>
		forward_memory_iter(forward_memory_iter<OtherValue> const & other,
			typename boost::enable_if<
			boost::is_convertible<OtherValue*,T*>
			, enabler
			>::type = enabler())
			: data(other.data)
		{}

	protected:
	private:
		friend class boost::iterator_core_access;
		template <class> friend class forward_memory_iter;

		void increment() { ++data; }
		template<class OtherValue>
		bool equal(forward_memory_iter<OtherValue> const &other) const
		{
			return other.data == data;
		}

		T& dereference() const { return *data; }

		T* data;

		

	};

	template<class T>
	class reverse_memory_iter : 
		public boost::iterator_facade<reverse_memory_iter<T>,typename T, boost::forward_traversal_tag >
	{
	public:
		struct enabler {};

		reverse_memory_iter() : T(0)
		{}
		explicit reverse_memory_iter(T* p) : data(p)
		{}
		template<class OtherValue>
		reverse_memory_iter(reverse_memory_iter<OtherValue> const & other,
			typename boost::enable_if<
			boost::is_convertible<OtherValue*,T*>
			, enabler
			>::type = enabler())
			: data(other.data)
		{}

	protected:
	private:
		friend class boost::iterator_core_access;
		template <class> friend class reverse_memory_iter;

		void increment() { --data; }
		template<class OtherValue>
		bool equal(reverse_memory_iter<OtherValue> const &other) const
		{
			return other.data == data;
		}

		T& dereference() const { return *data; }

		T* data;

		

	};
}

#endif