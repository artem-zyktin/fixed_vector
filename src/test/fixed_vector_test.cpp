#include <fv/fixed_vector.hpp>

#include <gtest/gtest.h>

namespace fixed_vector_test
{

using namespace fv;

TEST(default_allocator, allocate_deallocate)
{
	default_allocator<int> al;
	int* arr = al.allocate(3);

	EXPECT_TRUE(arr != nullptr);

	al.deallocate(arr, 3);
}

TEST(fixed_vector, ctor_and_sizes)
{
	using fvector_int = fixed_vector<int>;

	constexpr fvector_int::size_type CAPACITY = 10;

	fvector_int vec(CAPACITY);

	EXPECT_EQ(vec.size(), 0);
	EXPECT_EQ(vec.capacity(), CAPACITY);
	EXPECT_TRUE(vec.empty());
	EXPECT_FALSE(vec.full());
}

TEST(fixed_vector, push_back_and_access)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 5;

	fvector_int vec(CAPACITY);

	vec.push_back(1);
	vec.push_back(2);

	EXPECT_EQ(vec.at(0), 1);
	EXPECT_EQ(vec.at(1), 2);
	EXPECT_EQ(vec[0], 1);
	EXPECT_EQ(vec[1], 2);
}

struct Mok1
{
	int x, y;
	Mok1(int a, int b) : x(a), y(b) {}
};

TEST(fixed_vector, emplace_back)
{
	using fvector_Mok1 = fixed_vector<Mok1>;
	constexpr fvector_Mok1::size_type CAPACITY = 4;
	fvector_Mok1 vec (CAPACITY);

	vec.emplace_back(10, 20);
	EXPECT_EQ(vec.size(), 1);
	EXPECT_EQ(vec.at(0).x, 10);
	EXPECT_EQ(vec.at(0).y, 20);
}

TEST(fixed_vector, remove_and_swap)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 5;

	fvector_int vec (CAPACITY);

	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	vec.push_back(4);

	EXPECT_EQ(vec.size(), 4);
	
	vec.remove(1); // remove item '2'

	EXPECT_EQ(vec.size(), 3);
	EXPECT_EQ(vec[0], 1);
	EXPECT_EQ(vec[1], 4);
	EXPECT_EQ(vec[2], 3);	
}

TEST(fixed_vector, clean_and_reset_size)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 5;

	fvector_int vec(CAPACITY);

	vec.push_back(7);
	vec.push_back(8);

	EXPECT_EQ(vec.size(), 2);

	vec.clean();

	EXPECT_EQ(vec.size(), 0);
	EXPECT_TRUE(vec.empty());
}

TEST(fixed_vector, copy_ctor)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 5;
	
	fvector_int vec1 (CAPACITY);
	vec1.push_back(1);
	vec1.push_back(2);

	fvector_int vec2 = vec1;

	EXPECT_EQ(vec2.size(), 2);
	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
}

TEST(fixed_vector, move_ctor)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 5;

	fvector_int vec1(CAPACITY);
	vec1.push_back(1);
	vec1.push_back(2);

	fvector_int vec2 = std::move(vec1);

	EXPECT_EQ(vec1.size(), 0);
	EXPECT_EQ(vec1.capacity(), 0);
	EXPECT_EQ(vec2.size(), 2);
	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
}

TEST(fixed_vector, copy_assignment)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 3;

	fvector_int vec1(CAPACITY);
	vec1.push_back(1);
	vec1.push_back(2);

	fvector_int vec2 (5);
	vec2 = vec1;


	EXPECT_EQ(vec2.size(), 2);
	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
}

TEST(fixed_vector, move_assignment)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 3;

	fvector_int vec1(CAPACITY);
	vec1.push_back(1);
	vec1.push_back(2);

	fvector_int vec2(5);
	vec2 = std::move(vec1);

	EXPECT_EQ(vec1.size(), 0);
	EXPECT_EQ(vec1.capacity(), 0);
	EXPECT_EQ(vec2.size(), 2);
	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
}
	
TEST(fixed_vector, iterators)
{
	using fvector_int = fixed_vector<int>;
	constexpr fvector_int::size_type CAPACITY = 3;

	fvector_int vec (CAPACITY);

	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	int sum = 0;
	for (int v : vec)
		sum += v;

	EXPECT_EQ(sum, 6);
}

}