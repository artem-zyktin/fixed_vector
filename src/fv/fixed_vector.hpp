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

	default_allocator() = default;
	~default_allocator() = default;

	value_type* allocate(size_type size);
	void deallocate(value_type* p, size_type size);

};
static_assert(allocator_concept<default_allocator<void>, void>);

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

	fixed_vector() noexcept;
	fixed_vector(size_type size);
	~fixed_vector() noexcept;

	fixed_vector(const fixed_vector& other);
	fixed_vector& operator=(const fixed_vector& other);

	fixed_vector(fixed_vector&& other) noexcept;
	fixed_vector& operator=(fixed_vector&& other) noexcept;

	void push_back(cref_type item);
	void push_back(rref_type item);

	template<class... arg_type>
	void emplace_back(arg_type&&... arg);

	void remove(size_type item);

	void clear();

	cref_type operator=(size_type index) const;
	ref_type  operator=(size_type index);

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
