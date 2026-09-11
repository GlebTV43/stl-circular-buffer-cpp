#include <circular_buffer.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <vector>

TEST(CircularBufferThrowTest, frontFromEmptyBufferThrows) {
    circular_buffer<int> cb(3);

    ASSERT_THROW(cb.front(), std::out_of_range);
    ASSERT_TRUE(cb.empty());
}

TEST(CircularBufferThrowTest, backFromEmptyBufferThrows) {
    circular_buffer<int> cb(3);

    ASSERT_THROW(cb.back(), std::out_of_range);
    ASSERT_TRUE(cb.empty());
}

TEST(CircularBufferThrowTest, popFrontFromEmptyBufferThrows) {
    circular_buffer<int> cb(3);

    ASSERT_THROW(cb.pop_front(), std::out_of_range);
    ASSERT_TRUE(cb.empty());
}

TEST(CircularBufferThrowTest, popBackFromEmptyBufferThrows) {
    circular_buffer<int> cb(3);

    ASSERT_THROW(cb.pop_back(), std::out_of_range);
    ASSERT_TRUE(cb.empty());
}

TEST(CircularBufferThrowTest, atOutsideBufferThrows) {
    circular_buffer<int> cb = {1, 2, 3};

    ASSERT_THROW(cb.at(3), std::out_of_range);
    ASSERT_THROW(cb.at(100), std::out_of_range);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, constAtOutsideBufferThrows) {
    const circular_buffer<int> cb = {1, 2, 3};

    ASSERT_THROW(cb.at(3), std::out_of_range);
    ASSERT_THROW(cb.at(100), std::out_of_range);
}

TEST(CircularBufferThrowTest, pushBackIntoZeroCapacityBufferThrows) {
    circular_buffer<int> cb;

    ASSERT_THROW(cb.push_back(1), std::length_error);
    ASSERT_TRUE(cb.empty());
    ASSERT_EQ(cb.capacity(), 0);
}

TEST(CircularBufferThrowTest, pushFrontIntoZeroCapacityBufferThrows) {
    circular_buffer<int> cb;

    ASSERT_THROW(cb.push_front(1), std::length_error);
    ASSERT_TRUE(cb.empty());
    ASSERT_EQ(cb.capacity(), 0);
}

TEST(CircularBufferThrowTest, fixedReserveOverCapacityThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.reserve(10), std::length_error);
    ASSERT_EQ(cb.capacity(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedResizeOverCapacityThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.resize(5), std::length_error);
    ASSERT_EQ(cb.capacity(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedResizeWithValueOverCapacityThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.resize(5, 10), std::length_error);
    ASSERT_EQ(cb.capacity(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedAssignCountOverCapacityThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.assign(5, 10), std::length_error);
    ASSERT_EQ(cb.capacity(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedAssignRangeOverCapacityThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};
    std::vector<int> values = {4, 5, 6, 7};

    ASSERT_THROW(cb.assign(values.begin(), values.end()), std::length_error);
    ASSERT_EQ(cb.capacity(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedAssignInitializerListOverCapacityThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.assign({4, 5, 6, 7}), std::length_error);
    ASSERT_EQ(cb.capacity(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedInsertValueWithoutSpaceThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.insert(cb.cbegin() + 1, 10), std::length_error);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedInsertCountWithoutSpaceThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.insert(cb.cbegin() + 1, 2, 10), std::length_error);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedInsertRangeWithoutSpaceThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};
    std::vector<int> values = {4, 5};

    ASSERT_THROW(
        cb.insert(cb.cbegin() + 1, values.begin(), values.end()),
        std::length_error
    );
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, fixedInsertInitializerListWithoutSpaceThrows) {
    circular_buffer<int, false> cb = {1, 2, 3};

    ASSERT_THROW(cb.insert(cb.cbegin() + 1, {4, 5}), std::length_error);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, insertWithForeignIteratorThrows) {
    circular_buffer<int, true> first = {1, 2, 3};
    circular_buffer<int, true> second = {4, 5, 6};

    ASSERT_THROW(first.insert(second.cbegin(), 10), std::out_of_range);
    ASSERT_THAT(first, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, eraseWithForeignIteratorThrows) {
    circular_buffer<int> first = {1, 2, 3};
    circular_buffer<int> second = {4, 5, 6};

    ASSERT_THROW(first.erase(second.cbegin()), std::out_of_range);
    ASSERT_THAT(first, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, eraseRangeWithForeignIteratorThrows) {
    circular_buffer<int> first = {1, 2, 3};
    circular_buffer<int> second = {4, 5, 6};

    ASSERT_THROW(first.erase(first.cbegin(), second.cend()), std::out_of_range);
    ASSERT_THAT(first, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, eraseEndThrows) {
    circular_buffer<int> cb = {1, 2, 3};

    ASSERT_THROW(cb.erase(cb.cend()), std::out_of_range);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferThrowTest, eraseReversedRangeThrows) {
    circular_buffer<int> cb = {1, 2, 3, 4, 5};

    ASSERT_THROW(
        cb.erase(cb.cbegin() + 4, cb.cbegin() + 1), std::out_of_range);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

class ThrowOnCopyForModifier {
public:
    static inline bool ThrowFromCopy = false;

    int value = 0;

    explicit ThrowOnCopyForModifier(int new_value) : value(new_value) {}

    ThrowOnCopyForModifier(const ThrowOnCopyForModifier& other) : value(other.value) 
    {
        if (ThrowFromCopy) 
        {
            throw std::runtime_error("copy failed");
        }
    }

    ThrowOnCopyForModifier(ThrowOnCopyForModifier&& other) noexcept(false) : value(other.value) {}

    ThrowOnCopyForModifier& operator=(const ThrowOnCopyForModifier&) = default;
    ThrowOnCopyForModifier& operator=(ThrowOnCopyForModifier&&) = default;
};

class CircularBufferElementThrowTest : public testing::Test {
public:
    void SetUp() override {
        ThrowOnCopyForModifier::ThrowFromCopy = false;
    }

    void TearDown() override {
        ThrowOnCopyForModifier::ThrowFromCopy = false;
    }
};

TEST_F(CircularBufferElementThrowTest, reserveCopyExceptionPreservesBuffer) {
    circular_buffer<ThrowOnCopyForModifier, true> cb(3);
    cb.push_back(ThrowOnCopyForModifier(1));
    cb.push_back(ThrowOnCopyForModifier(2));
    cb.push_back(ThrowOnCopyForModifier(3));

    ThrowOnCopyForModifier::ThrowFromCopy = true;

    ASSERT_THROW(cb.reserve(10), std::runtime_error);
    ASSERT_EQ(cb.size(), 3);
    ASSERT_EQ(cb.capacity(), 3);
    ASSERT_EQ(cb[0].value, 1);
    ASSERT_EQ(cb[1].value, 2);
    ASSERT_EQ(cb[2].value, 3);
}

TEST_F(CircularBufferElementThrowTest, insertCopyExceptionPreservesBuffer) {
    circular_buffer<ThrowOnCopyForModifier, true> cb(5);
    cb.push_back(ThrowOnCopyForModifier(1));
    cb.push_back(ThrowOnCopyForModifier(2));
    ThrowOnCopyForModifier value(3);

    ThrowOnCopyForModifier::ThrowFromCopy = true;

    ASSERT_THROW(cb.insert(cb.cbegin() + 1, value), std::runtime_error);
    ASSERT_EQ(cb.size(), 2);
    ASSERT_EQ(cb[0].value, 1);
    ASSERT_EQ(cb[1].value, 2);
}

TEST_F(CircularBufferElementThrowTest, assignCopyExceptionPreservesBuffer) {
    circular_buffer<ThrowOnCopyForModifier, true> cb(5);
    cb.push_back(ThrowOnCopyForModifier(1));
    cb.push_back(ThrowOnCopyForModifier(2));
    ThrowOnCopyForModifier value(3);

    ThrowOnCopyForModifier::ThrowFromCopy = true;

    ASSERT_THROW(cb.assign(4, value), std::runtime_error);
    ASSERT_EQ(cb.size(), 2);
    ASSERT_EQ(cb[0].value, 1);
    ASSERT_EQ(cb[1].value, 2);
}