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
		return reinterpret_cast<value_type*>(::operator new[](n_bytes_(size)));
	}

	inline void deallocate(value_type* p, size_type size)
	{
		assert(p);
		::operator delete[](reinterpret_cast<void*>(p), n_bytes_(size));
	}

private:
	constexpr size_type n_bytes_(size_type size) { return size * sizeof(value_type); }
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
	static_assert(std::is_nothrow_move_constructible_v<allocator_t>, "Allocator must be nothrow move constructible");
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

private:
	allocator_t allocator_ = {};

	ptr_type data_ = nullptr;
	size_type capacity_ = 0;
	size_type size_ = 0;

	void release_();

	template<class fv_t, class tranfsfer_fn>
		requires std::same_as<std::remove_cvref_t<fv_t>, fixed_vector<T, allocator_t>>
	fixed_vector<T, allocator_t>& copy_or_move_assignment_(fv_t&& other, tranfsfer_fn&& transfer_strategy);
};

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::fixed_vector(size_type capacity)
{
	assert(capacity > 0);

	data_ = allocator_.allocate(capacity);
	assert(data_);

	capacity_ = capacity;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::~fixed_vector() noexcept
{
	release_();
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::fixed_vector(const fixed_vector& other)
	: allocator_(other.allocator_)
	, data_(allocator_.allocate(other.capacity_))
	, capacity_(other.capacity_)
	, size_(other.size_)
{
	assert(data_);
	std::uninitialized_copy_n(other.data_, other.size_, data_);
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::fixed_vector(fixed_vector&& other) noexcept
	: allocator_(std::move(other.allocator_))
	, data_(std::exchange(other.data_, nullptr))
	, capacity_(std::exchange(other.capacity_, 0))
	, size_(std::exchange(other.size_, 0))
{
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>& fixed_vector<T, allocator_t>::operator=(const fixed_vector& other)
{
	return copy_or_move_assignment_(other, [](const fixed_vector& other, ptr_type dst){
		std::copy_n(other.data_, other.size_, dst);
	});
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>& fixed_vector<T, allocator_t>::operator=(fixed_vector&& other) noexcept
{
	return  copy_or_move_assignment_(other, [](fixed_vector&& other, ptr_type dst){
		std::move_n(other.data_, other.size_, dst);

		other.data_ = nullptr;
		other.capacity_ = 0;
		other.size_ = 0;
	});
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline void fixed_vector<T, allocator_t>::push_back(cref_type item)
{
	assert(data_);
	assert(size_ < capacity_);

	std::uninitialized_copy_n(&item, 1, data_+size_);
	++size_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline void fixed_vector<T, allocator_t>::push_back(rref_type item)
{
	assert(data_);
	assert(size_ < capacity_);

	std::construct_at(&data_[size_], std::move(item));
	++size_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline void fixed_vector<T, allocator_t>::release_()
{
	if (data_)
	{
		clean();

		allocator_.deallocate(data_, capacity_);
		data_ = nullptr;
		capacity_ = 0;
	}
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline void fixed_vector<T, allocator_t>::remove(size_type index)
{
	assert(data_);
	if (index != size_-1)
		std::swap(data_[index], data_[size_-1]);

	std::destroy_at(&data_[size_-1]);
	--size_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline void fixed_vector<T, allocator_t>::clean()
{
	std::destroy_n(data_, size_);
	size_ = 0;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::cref_type fixed_vector<T, allocator_t>::operator[](size_type index) const
{
	return data_[index];
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::ref_type fixed_vector<T, allocator_t>::operator[](size_type index)
{
	return data_[index];
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::cref_type fixed_vector<T, allocator_t>::at(size_type index) const
{
	assert(index < size_);
	return data_[index];
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::ref_type fixed_vector<T, allocator_t>::at(size_type index)
{
	assert(index < size_);
	return data_[index];
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::size_type fixed_vector<T, allocator_t>::size() const
{
	return size_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::size_type fixed_vector<T, allocator_t>::capacity() const
{
	return capacity_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline bool fixed_vector<T, allocator_t>::full() const
{
	return size_ == capacity_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline bool fixed_vector<T, allocator_t>::empty() const
{
	return size_ == 0;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::iterator fixed_vector<T, allocator_t>::begin()
{
	return data_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::iterator fixed_vector<T, allocator_t>::end()
{
	return data_ + size_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::const_iterator fixed_vector<T, allocator_t>::cbegin() const
{
	return data_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::const_iterator fixed_vector<T, allocator_t>::cend() const
{
	return data_ + size_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::const_iterator fixed_vector<T, allocator_t>::begin() const
{
	return cbegin();
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::const_iterator fixed_vector<T, allocator_t>::end() const
{
	return cend();
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::reverse_iterator fixed_vector<T, allocator_t>::rbegin()
{
	return reverse_iterator(end());
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::reverse_iterator fixed_vector<T, allocator_t>::rend()
{
	return reverse_iterator(begin());
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::const_reverse_iterator fixed_vector<T, allocator_t>::crbegin() const
{
	return rbegin();
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
inline fixed_vector<T, allocator_t>::const_reverse_iterator fixed_vector<T, allocator_t>::crend() const
{
	return rend();
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
template<class ...arg_type>
inline void fixed_vector<T, allocator_t>::emplace_back(arg_type&& ...arg)
{
	assert(data_);
	assert(size_ < capacity_);
	std::construct_at(&data_[size_], std::forward<arg_type>(arg)...);
	++size_;
}

template<class T, allocator_concept<std::remove_cvref_t<T>> allocator_t>
template<class fv_t, class tranfsfer_fn>
	requires std::same_as<std::remove_cvref_t<fv_t>, fixed_vector<T, allocator_t>>
inline fixed_vector<T, allocator_t>& fixed_vector<T, allocator_t>::copy_or_move_assignment_(fv_t&& other, tranfsfer_fn&& transfer_strategy)
{
	if (this == &other)
		return;

	if constexpr (std::is_rvalue_reference_v<decltype(other)>)
		allocator_ = std::move(other.allocator_);
	else
		allocator_ = other.allocator_;

	if (other.capacity_ > capacity_)
	{
		release_();
		data_ = allocator_.allocate(other.capacity_);
		assert(data_);

		capacity_ = other.capacity_;
	}
	else
	{
		clean();
	}

	size_ = other.size_;
	transfer_strategy(std::forward<fv_t>(other), data_);

	return *this;
}

}
