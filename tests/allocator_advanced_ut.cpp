#include <circular_buffer.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstddef>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

class AllocatorState {
public:
    static inline int AllocationCount = 0;
    static inline int DeallocationCount = 0;
    static inline int WrongDeallocationCount = 0;
    static inline std::unordered_map<void*, int> Owners;

    static void Reset() {
        AllocationCount = 0;
        DeallocationCount = 0;
        WrongDeallocationCount = 0;
        Owners.clear();
    }
};

template<typename T, bool CopyPropagation, bool MovePropagation,
         bool SwapPropagation, bool AlwaysEqual = false>
class StatefulAllocator {
public:
    using value_type = T;
    using propagate_on_container_copy_assignment = std::bool_constant<CopyPropagation>;
    using propagate_on_container_move_assignment = std::bool_constant<MovePropagation>;
    using propagate_on_container_swap = std::bool_constant<SwapPropagation>;
    using is_always_equal = std::bool_constant<AlwaysEqual>;

    template<typename U>
    struct rebind {
        using other = StatefulAllocator<U, CopyPropagation, MovePropagation,
                                        SwapPropagation, AlwaysEqual>;
    };

    int id = 0;

    StatefulAllocator() = default;

    explicit StatefulAllocator(int new_id)
        : id(new_id) {}

    template<typename U>
    StatefulAllocator(const StatefulAllocator<U, CopyPropagation, MovePropagation,
                                               SwapPropagation, AlwaysEqual>& other)
        : id(other.id) {}

    T* allocate(std::size_t count) {
        T* result = std::allocator<T>{}.allocate(count);
        AllocatorState::Owners[result] = id;
        ++AllocatorState::AllocationCount;
        return result;
    }

    void deallocate(T* pointer, std::size_t count) noexcept {
        auto owner = AllocatorState::Owners.find(pointer);
        if (owner == AllocatorState::Owners.end() ||
            (!AlwaysEqual && owner->second != id)) {
            ++AllocatorState::WrongDeallocationCount;
        } else {
            AllocatorState::Owners.erase(owner);
        }
        ++AllocatorState::DeallocationCount;
        std::allocator<T>{}.deallocate(pointer, count);
    }

    StatefulAllocator select_on_container_copy_construction() const {
        return StatefulAllocator(id + 100);
    }

    template<typename U>
    bool operator==(const StatefulAllocator<U, CopyPropagation, MovePropagation,
                                             SwapPropagation, AlwaysEqual>& other) const {
        return AlwaysEqual || id == other.id;
    }
};

class CircularBufferAllocatorAdvancedTest : public testing::Test {
public:
    void SetUp() override {
        AllocatorState::Reset();
    }

    void TearDown() override {
        ASSERT_EQ(AllocatorState::WrongDeallocationCount, 0);
        ASSERT_TRUE(AllocatorState::Owners.empty());
        ASSERT_EQ(AllocatorState::AllocationCount, AllocatorState::DeallocationCount);
    }
};

TEST_F(CircularBufferAllocatorAdvancedTest, copyConstructorSelectsAllocator) {
    using Allocator = StatefulAllocator<int, false, false, false>;
    using Buffer = circular_buffer<int, false, Allocator>;

    Buffer source({1, 2, 3}, Allocator(7));
    Buffer copy(source);

    ASSERT_EQ(source.get_allocator().id, 7);
    ASSERT_EQ(copy.get_allocator().id, 107);
    ASSERT_THAT(copy, testing::ElementsAre(1, 2, 3));
}

TEST_F(CircularBufferAllocatorAdvancedTest, copyAssignmentWithoutPropagationKeepsAllocator) {
    using Allocator = StatefulAllocator<int, false, false, false>;
    using Buffer = circular_buffer<int, false, Allocator>;

    Buffer source({1, 2, 3}, Allocator(1));
    Buffer destination({4, 5}, Allocator(2));

    destination = source;

    ASSERT_EQ(destination.get_allocator().id, 2);
    ASSERT_THAT(destination, testing::ElementsAre(1, 2, 3));
}

TEST_F(CircularBufferAllocatorAdvancedTest, copyAssignmentWithPropagationChangesAllocator) {
    using Allocator = StatefulAllocator<int, true, false, false>;
    using Buffer = circular_buffer<int, false, Allocator>;

    Buffer source({1, 2, 3}, Allocator(1));
    Buffer destination({4, 5}, Allocator(2));

    destination = source;

    ASSERT_EQ(destination.get_allocator().id, 1);
    ASSERT_THAT(destination, testing::ElementsAre(1, 2, 3));
}

TEST_F(CircularBufferAllocatorAdvancedTest, moveAssignmentWithoutPropagationKeepsAllocator) {
    using Allocator = StatefulAllocator<int, false, false, false>;
    using Buffer = circular_buffer<int, false, Allocator>;

    Buffer source({1, 2, 3, 4}, Allocator(1));
    Buffer destination({5, 6}, Allocator(2));
    const auto source_capacity = source.capacity();

    destination = std::move(source);

    ASSERT_EQ(destination.get_allocator().id, 2);
    ASSERT_THAT(destination, testing::ElementsAre(1, 2, 3, 4));
    ASSERT_TRUE(source.empty());
    ASSERT_EQ(source.capacity(), source_capacity);
}

TEST_F(CircularBufferAllocatorAdvancedTest, moveAssignmentWithPropagationChangesAllocator) {
    using Allocator = StatefulAllocator<int, false, true, false>;
    using Buffer = circular_buffer<int, false, Allocator>;

    Buffer source({1, 2, 3, 4}, Allocator(1));
    Buffer destination({5, 6}, Allocator(2));

    destination = std::move(source);

    ASSERT_EQ(destination.get_allocator().id, 1);
    ASSERT_THAT(destination, testing::ElementsAre(1, 2, 3, 4));
    ASSERT_TRUE(source.empty());
    ASSERT_EQ(source.capacity(), 0);
}

TEST_F(CircularBufferAllocatorAdvancedTest, swapWithPropagationSwapsAllocators) {
    using Allocator = StatefulAllocator<int, false, false, true>;
    using Buffer = circular_buffer<int, false, Allocator>;

    Buffer first({1, 2, 3}, Allocator(1));
    Buffer second({4, 5}, Allocator(2));

    swap(first, second);

    ASSERT_EQ(first.get_allocator().id, 2);
    ASSERT_EQ(second.get_allocator().id, 1);
    ASSERT_THAT(first, testing::ElementsAre(4, 5));
    ASSERT_THAT(second, testing::ElementsAre(1, 2, 3));
}

TEST_F(CircularBufferAllocatorAdvancedTest, selfAssignmentsDoNotLoseStorage) {
    using Allocator = StatefulAllocator<int, false, false, false>;
    using Buffer = circular_buffer<int, false, Allocator>;

    Buffer cb({1, 2, 3}, Allocator(1));

    cb = cb;
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));

    auto* same_buffer = &cb;
    cb = std::move(*same_buffer);
    ASSERT_THAT(cb, testing::ElementsAre(1, 2, 3));
    ASSERT_EQ(cb.get_allocator().id, 1);
}
