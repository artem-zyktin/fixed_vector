#include <cstdint>
#include <concepts>
#include <type_traits>
#include <cassert>
#include <memory>
#include <utility>
#include <iterator>

namespace fv
{

template<class allocator_type, class value_type>
concept allocator_concept = std::default_initializable<allocator_type> &&
requires (allocator_type allocator, value_type* p, uint32_t size)
{
	typename allocator_type::size_type;
	{ allocator.allocate(size) } -> std::convertible_to<value_type*>;
	{ allocator.deallocate(p, size) };
};

template<class T>
class default_allocator
{
public:
	using value_type = std::remove_cvref_t<T>;
	using size_type = uint32_t;

	default_allocator() noexcept = default;
	~default_allocator() noexcept = default;
	default_allocator(const default_allocator&) noexcept = default;
	default_allocator(default_allocator&&) noexcept = default;
	default_allocator& operator=(const default_allocator&) noexcept = default;
	default_allocator& operator=(default_allocator&&) noexcept = default;

	inline value_type* allocate(size_type size)
	{
		return reinterpret_cast<value_type*>(::operator new[](size*sizeof(value_type));
	}

	inline void deallocate(value_type* p, size_type size)
	{
		assert(p);
		::operator delete[](reinterpret_cast<void*>(p), size);
	}

};
static_assert(allocator_concept<default_allocator<void>, void>);

/// <summary>
/// Fixed-capacity vector. Memory is allocated once on construction and never reallocated.
/// May allocate a new buffer only when copying or moving from another vector with larger capacity.
/// </summary>
/// <typeparam name="T">Items type</typeparam>
/// <typeparam name="allocator_t">Allocator</typeparam>
template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t = default_allocator<std::remove_cvref_t<T>>>
class fixed_vector
{
public:
	using value_type = std::remove_cvref_t<T>;

	using ref_type   = value_type&;
	using cref_type  = value_type const&;
	using rref_type  = value_type&&;

	using ptr_type   = value_type*;
	using cptr_type  = value_type const*;

	using iterator = ptr_type;
	using const_iterator = cptr_type;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	using size_type = uint32_t;

	/// <summary>
	/// Ctor
	/// </summary>
	/// <param name="capacity"> - size of allocated memory block</param>
	fixed_vector(size_type capacity);
	~fixed_vector() noexcept;

	/// <summary>
	/// Copy ctor
	/// </summary>
	fixed_vector(const fixed_vector& other);

	/// <summary>
	/// Move ctor
	/// </summary>
	fixed_vector(fixed_vector&& other) noexcept;

	/// <summary>
	/// Copy assignment operator
	/// </summary>
	fixed_vector& operator=(const fixed_vector& other);

	/// <summary>
	/// Move assignment operator
	/// </summary>
	fixed_vector& operator=(fixed_vector&& other) noexcept;

	/// <summary>
	/// Add item to the end (copying)
	/// </summary>
	void push_back(cref_type item);

	/// <summary>
	/// Add item to the end (moving)
	/// </summary>
	void push_back(rref_type item);


	/// <summary>
	/// Emplacing item to the end
	/// </summary>
	template<class... arg_type>
	void emplace_back(arg_type&&... arg);

	/// <summary>
	/// Remove item. If index != size()-1, then item will be swapped with the last item and only then last item destroyed
	/// </summary>
	/// <param name="index"> - index of removing item</param>
	void remove(size_type index);

	/// <summary>
	/// Destroy all items
	/// </summary>
	void clean();

	cref_type operator[](size_type index) const;
	ref_type  operator[](size_type index);

	cref_type at(size_type index) const;
	ref_type  at(size_type index);

	size_type size() const;
	size_type capacity() const;

	bool full() const;
	bool empty() const;

	iterator begin();
	iterator end();

	const_iterator cbegin() const;
	const_iterator cend() const;

	const_iterator begin() const;
	const_iterator end() const;

	reverse_iterator rbegin();
	reverse_iterator rend();

	const_reverse_iterator crbegin() const;
	const_reverse_iterator crend() const;
};

}
