#include <circular_buffer.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <utility>

TEST(CircularBufferRuleOfFiveTest, copyConstructorCopiesWrappedBuffer) {
    circular_buffer<int> source(5);
    source.push_back(1);
    source.push_back(2);
    source.push_back(3);
    source.push_back(4);
    source.push_back(5);
    source.pop_front();
    source.push_back(6);

    circular_buffer<int> copy(source);
    copy[0] = 100;

    ASSERT_THAT(source, testing::ElementsAre(2, 3, 4, 5, 6));
    ASSERT_THAT(copy, testing::ElementsAre(100, 3, 4, 5, 6));
    ASSERT_EQ(copy.capacity(), source.capacity());
}

TEST(CircularBufferRuleOfFiveTest, copyAssignmentMakesIndependentCopy) {
    circular_buffer<std::string> source = {"one", "two", "three"};
    circular_buffer<std::string> destination(8);
    destination.push_back("old");

    destination = source;
    destination[1] = "changed";

    ASSERT_THAT(source, testing::ElementsAre("one", "two", "three"));
    ASSERT_THAT(destination, testing::ElementsAre("one", "changed", "three"));
}

TEST(CircularBufferRuleOfFiveTest, selfCopyAssignmentPreservesBuffer) {
    circular_buffer<int> cb = {1, 2, 3, 4};

    cb = cb;

    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4));
}

TEST(CircularBufferRuleOfFiveTest, moveConstructorTransfersStorage) {
    circular_buffer<int> source = {1, 2, 3, 4};

    circular_buffer<int> destination(std::move(source));

    ASSERT_THAT(destination, testing::ElementsAre(1, 2, 3, 4));
    ASSERT_TRUE(source.empty());
    ASSERT_EQ(source.capacity(), 0);
}

TEST(CircularBufferRuleOfFiveTest, moveAssignmentTransfersStorage) {
    circular_buffer<int> source = {1, 2, 3, 4};
    circular_buffer<int> destination = {5, 6};

    destination = std::move(source);

    ASSERT_THAT(destination, testing::ElementsAre(1, 2, 3, 4));
    ASSERT_TRUE(source.empty());
    ASSERT_EQ(source.capacity(), 0);
}

TEST(CircularBufferRuleOfFiveTest, selfMoveAssignmentPreservesBuffer) {
    circular_buffer<int> cb = {1, 2, 3, 4};
    auto* same_buffer = &cb;

    cb = std::move(*same_buffer);

    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4));
}

TEST(CircularBufferRuleOfFiveTest, initializerListAssignmentReplacesValues) {
    circular_buffer<int> cb(5);
    cb.push_back(10);
    cb.push_back(20);

    cb = {1, 2, 3, 4, 5};

    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3, 4, 5));
}