#include <cstdint>
#include <concepts>
#include <type_traits>
#include <cassert>
#include <memory>
#include <utility>

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

}
