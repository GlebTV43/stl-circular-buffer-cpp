#include <circular_buffer.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <string>
#include <vector>

TEST(CircularBufferInsertEraseTest, insertIntoWrappedStorage) {
    circular_buffer<int> cb(8);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.push_back(5);
    cb.push_back(6);
    cb.pop_front();
    cb.pop_front();
    cb.pop_front();
    cb.push_back(7);
    cb.push_back(8);
    cb.push_back(9);

    auto result = cb.insert(cb.cbegin() + 2, 42);

    ASSERT_EQ(*result, 42);
    ASSERT_THAT(cb, testing::ElementsAre(4, 5, 42, 6, 7, 8, 9));
}

TEST(CircularBufferInsertEraseTest, eraseFromWrappedStorage) {
    circular_buffer<int> cb(8);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.push_back(5);
    cb.push_back(6);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(7);
    cb.push_back(8);
    cb.push_back(9);

    auto result = cb.erase(cb.cbegin() + 2, cb.cbegin() + 5);

    ASSERT_EQ(*result, 8);
    ASSERT_THAT(cb, testing::ElementsAre(3, 4, 8, 9));
}

TEST(CircularBufferInsertEraseTest, eraseEmptyRangeChangesNothing) {
    circular_buffer<int> cb = {1, 2, 3, 4};

    auto result = cb.erase(cb.cbegin() + 2, cb.cbegin() + 2);

    ASSERT_EQ(result, cb.begin() + 2);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4));
}

TEST(CircularBufferInsertEraseTest, eraseAllElements) {
    circular_buffer<int> cb = {1, 2, 3, 4};

    auto result = cb.erase(cb.cbegin(), cb.cend());

    ASSERT_EQ(result, cb.end());
    ASSERT_TRUE(cb.empty());
    ASSERT_EQ(cb.capacity(), 4);
}

TEST(CircularBufferInsertEraseTest, insertAtBeginAndEnd) {
    circular_buffer<int, true> cb = {2, 3, 4};

    auto first = cb.insert(cb.cbegin(), 1);
    auto last = cb.insert(cb.cend(), 5);

    ASSERT_EQ(*first, 1);
    ASSERT_EQ(*last, 5);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(CircularBufferInsertEraseTest, insertRangeCausesExpansion) {
    circular_buffer<int, true> cb = {1, 5};
    std::vector<int> values = {2, 3, 4};

    auto result = cb.insert(cb.cbegin() + 1, values.begin(), values.end());

    ASSERT_EQ(*result, 2);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(CircularBufferInsertEraseTest, fixedInsertWithoutFreeSpaceThrows) {
    circular_buffer<int> cb = {1, 2, 3};

    ASSERT_THROW(cb.insert(cb.cbegin() + 1, 4), std::length_error);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(ExtendableBufferAdvancedTest, expansionPreservesWrappedOrder) {
    circular_buffer<int, true> cb(4);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(5);
    cb.push_back(6);

    cb.push_back(7);

    ASSERT_EQ(cb.size(), 5);
    ASSERT_GE(cb.capacity(), 5);
    ASSERT_THAT(cb, testing::ElementsAre(3, 4, 5, 6, 7));
}

TEST(ExtendableBufferAdvancedTest, reservePreservesWrappedOrder) {
    circular_buffer<std::string, true> cb(4);
    cb.push_back("one");
    cb.push_back("two");
    cb.push_back("three");
    cb.push_back("four");
    cb.pop_front();
    cb.push_back("five");

    cb.reserve(20);

    ASSERT_EQ(cb.capacity(), 20);
    ASSERT_THAT(cb, testing::ElementsAre("two", "three", "four", "five"));
}

TEST(ExtendableBufferAdvancedTest, resizeAfterWrapAround) {
    circular_buffer<int, true> cb(4);
    cb.push_back(1);
    cb.push_back(2);
    cb.push_back(3);
    cb.push_back(4);
    cb.pop_front();
    cb.pop_front();
    cb.push_back(5);
    cb.push_back(6);

    cb.resize(7, 9);

    ASSERT_THAT(cb, testing::ElementsAre(3, 4, 5, 6, 9, 9, 9));

    cb.resize(2);

    ASSERT_THAT(cb, testing::ElementsAre(3, 4));
}

TEST(ExtendableBufferAdvancedTest, assignRangeExpandsBuffer) {
    circular_buffer<int, true> cb(2);
    std::vector<int> values = {1, 2, 3, 4, 5, 6};

    cb.assign(values.begin(), values.end());

    ASSERT_GE(cb.capacity(), values.size());
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(ExtendableBufferAdvancedTest, fixedResizeOverCapacityThrows) {
    circular_buffer<int> cb = {1, 2, 3};

    ASSERT_THROW(cb.resize(4, 0), std::length_error);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
}

TEST(CircularBufferModifierTest, reverse) {
    circular_buffer<int> cb = {1, 2, 3, 4, 5};

    cb.reverse();

    ASSERT_THAT(cb, testing::ElementsAre(5, 4, 3, 2, 1));
}

TEST(CircularBufferModifierTest, reverseWrappedBuffer) {
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

    cb.reverse();

    ASSERT_THAT(cb, testing::ElementsAre(7, 6, 5, 4, 3));
}

TEST(CircularBufferModifierTest, reverseEmptyBuffer) {
    circular_buffer<int> cb(5);

    cb.reverse();

    ASSERT_TRUE(cb.empty());
}

TEST(CircularBufferModifierTest, reverseOneElement) {
    circular_buffer<int> cb(1);
    cb.push_back(42);

    cb.reverse();

    ASSERT_THAT(cb, testing::ElementsAre(42));
}