#include <circular_buffer.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <vector>

TEST(CircularBufferEdgeTest, wrapAroundPushBack) {
    circular_buffer<int> cb(5);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.push_back(5);

    cb.pop_front();
    cb.pop_front();
    cb.push_back(6);
    cb.push_back(7);

    ASSERT_THAT(cb, testing::ElementsAre(3, 4, 5, 6, 7));
    ASSERT_EQ(cb.front(), 3);
    ASSERT_EQ(cb.back(), 7);
}

TEST(CircularBufferEdgeTest, wrapAroundPushFront) {
    circular_buffer<int> cb(5);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.push_back(5);

    cb.pop_back();
    cb.pop_back();
    cb.push_front(0);
    cb.push_front(-1);

    ASSERT_THAT(cb, testing::ElementsAre(-1, 0, 1, 2, 3));
    ASSERT_EQ(cb.front(), -1);
    ASSERT_EQ(cb.back(), 3);
}

TEST(CircularBufferEdgeTest, fullPushBackOverwritesOldest) {
    circular_buffer<int> cb(3);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.push_back(5);

    ASSERT_EQ(cb.size(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(3, 4, 5));
}

TEST(CircularBufferEdgeTest, fullPushFrontOverwritesNewest) {
    circular_buffer<int> cb(3);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_front(0);
    cb.push_front(-1);

    ASSERT_EQ(cb.size(), 3);
    ASSERT_THAT(cb, testing::ElementsAre(-1, 0, 1));
}

TEST(CircularBufferEdgeTest, zeroCapacityPushThrows) {
    circular_buffer<int> cb;

    ASSERT_THROW(cb.push_back(1), std::length_error);
    ASSERT_THROW(cb.push_front(1), std::length_error);
    ASSERT_TRUE(cb.empty());
}

TEST(CircularBufferEdgeTest, operationsOnEmptyBufferThrow) {
    circular_buffer<int> cb(3);

    ASSERT_THROW(cb.front(), std::out_of_range);
    ASSERT_THROW(cb.back(), std::out_of_range);
    ASSERT_THROW(cb.pop_front(), std::out_of_range);
    ASSERT_THROW(cb.pop_back(), std::out_of_range);
    ASSERT_THROW(cb.at(0), std::out_of_range);
}

TEST(CircularBufferEdgeTest, clearPreservesCapacityAndAllowsReuse) {
    circular_buffer<int> cb(5);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);

    cb.clear();

    ASSERT_TRUE(cb.empty());
    ASSERT_EQ(cb.size(), 0);
    ASSERT_EQ(cb.capacity(), 5);

    cb.push_front(4);
    cb.push_back(5);

    ASSERT_THAT(cb, testing::ElementsAre(4, 5));
}

TEST(CircularBufferEdgeTest, atChecksBounds) {
    circular_buffer<int> cb = {1, 2, 3};
    const circular_buffer<int>& const_cb = cb;

    ASSERT_EQ(cb.at(0), 1);
    ASSERT_EQ(const_cb.at(2), 3);
    ASSERT_THROW(cb.at(3), std::out_of_range);
    ASSERT_THROW(const_cb.at(100), std::out_of_range);
}

TEST(CircularBufferIteratorTest, iteratesThroughWrappedStorage) {
    circular_buffer<int> cb(5);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.push_back(5);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(6);
    cb.push_back(7);

    std::vector<int> values(cb.begin(), cb.end());

    ASSERT_THAT(values, testing::ElementsAre(3, 4, 5, 6, 7));
    ASSERT_EQ(cb.end() - cb.begin(), 5);
    ASSERT_EQ(*(cb.begin() + 3), 6);
    ASSERT_EQ(cb.begin()[4], 7);
}

TEST(CircularBufferIteratorTest, constIteratorsWorkAfterWrapAround) {
    circular_buffer<int> cb(4);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.pop_front();
    cb.push_back(5);

    const circular_buffer<int>& const_cb = cb;
    std::vector<int> values(const_cb.cbegin(), const_cb.cend());

    ASSERT_THAT(values, testing::ElementsAre(2, 3, 4, 5));
    ASSERT_EQ(const_cb.begin(), const_cb.cbegin());
    ASSERT_EQ(const_cb.end(), const_cb.cend());
}

TEST(CircularBufferIteratorTest, reverseIteratorsWorkAfterWrapAround) {
    circular_buffer<int> cb(4);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.pop_front();
    cb.push_back(5);

    const circular_buffer<int>& const_cb = cb;
    std::vector<int> values(const_cb.crbegin(), const_cb.crend());

    ASSERT_THAT(values, testing::ElementsAre(5, 4, 3, 2));
}

TEST(CircularBufferIteratorTest, iteratorCanChangeElements) {
    circular_buffer<int> cb = {1, 2, 3, 4};

    for (auto it = cb.begin(); it != cb.end(); ++it) {
        *it *= 2;
    }

    ASSERT_THAT(cb, testing::ElementsAre(2, 4, 6, 8));
}

TEST(CircularBufferIteratorTest, foreignIteratorIsRejected) {
    circular_buffer<int> first = {1, 2, 3};
    circular_buffer<int> second = {4, 5, 6};

    ASSERT_THROW(first.insert(second.cbegin(), 10), std::out_of_range);
    ASSERT_THROW(first.erase(second.cbegin()), std::out_of_range);
    ASSERT_THAT(first, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferIteratorTest, eraseEndIsRejected) {
    circular_buffer<int> cb = {1, 2, 3};

    ASSERT_THROW(cb.erase(cb.cend()), std::out_of_range);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}


