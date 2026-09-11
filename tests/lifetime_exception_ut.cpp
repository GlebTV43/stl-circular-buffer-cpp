#include <circular_buffer.h>

#include <gtest/gtest.h>

#include <stdexcept>

class TrackedValue {
public:
    static inline int Alive = 0;
    static inline int ConstructorCalled = 0;
    static inline int DestructorCalled = 0;

    int value = 0;

    explicit TrackedValue(int new_value)
        : value(new_value) {
        ++Alive;
        ++ConstructorCalled;
    }

    TrackedValue(const TrackedValue& other)
        : value(other.value) {
        ++Alive;
        ++ConstructorCalled;
    }

    TrackedValue(TrackedValue&& other) noexcept
        : value(other.value) {
        ++Alive;
        ++ConstructorCalled;
    }

    TrackedValue& operator=(const TrackedValue&) = default;
    TrackedValue& operator=(TrackedValue&&) noexcept = default;

    ~TrackedValue() {
        --Alive;
        ++DestructorCalled;
    }

    static void Reset() {
        Alive = 0;
        ConstructorCalled = 0;
        DestructorCalled = 0;
    }
};

class CircularBufferLifetimeTest : public testing::Test {
public:
    void SetUp() override {
        TrackedValue::Reset();
    }
};

TEST_F(CircularBufferLifetimeTest, popAndClearDestroyElementsImmediately) {
    {
        circular_buffer<TrackedValue> cb(4);
        cb.push_back(TrackedValue(1));
        cb.push_back(TrackedValue(2));
        cb.push_back(TrackedValue(3));

        ASSERT_EQ(TrackedValue::Alive, 3);

        cb.pop_front();
        ASSERT_EQ(TrackedValue::Alive, 2);

        cb.pop_back();
        ASSERT_EQ(TrackedValue::Alive, 1);

        cb.clear();
        ASSERT_EQ(TrackedValue::Alive, 0);
    }

    ASSERT_EQ(TrackedValue::Alive, 0);
    ASSERT_EQ(TrackedValue::ConstructorCalled, TrackedValue::DestructorCalled);
}

TEST_F(CircularBufferLifetimeTest, overwriteDestroysExactlyOneStoredElement) {
    {
        circular_buffer<TrackedValue> cb(2);
        cb.push_back(TrackedValue(1));
        cb.push_back(TrackedValue(2));

        ASSERT_EQ(TrackedValue::Alive, 2);

        cb.push_back(TrackedValue(3));

        ASSERT_EQ(TrackedValue::Alive, 2);
        ASSERT_EQ(cb.front().value, 2);
        ASSERT_EQ(cb.back().value, 3);
    }

    ASSERT_EQ(TrackedValue::Alive, 0);
    ASSERT_EQ(TrackedValue::ConstructorCalled, TrackedValue::DestructorCalled);
}

TEST_F(CircularBufferLifetimeTest, expansionDoesNotLeakObjects) {
    {
        circular_buffer<TrackedValue, true> cb(2);
        cb.push_back(TrackedValue(1));
        cb.push_back(TrackedValue(2));
        cb.push_back(TrackedValue(3));

        ASSERT_EQ(TrackedValue::Alive, 3);
    }

    ASSERT_EQ(TrackedValue::Alive, 0);
    ASSERT_EQ(TrackedValue::ConstructorCalled, TrackedValue::DestructorCalled);
}

class ThrowingValue {
public:
    static inline int Alive = 0;
    static inline int CopiesBeforeThrow = -1;

    int value = 0;

    explicit ThrowingValue(int new_value)
        : value(new_value) {
        ++Alive;
    }

    ThrowingValue(const ThrowingValue& other)
        : value(other.value) {
        if (CopiesBeforeThrow == 0) {
            throw std::runtime_error("copy failed");
        }
        if (CopiesBeforeThrow > 0) {
            --CopiesBeforeThrow;
        }
        ++Alive;
    }

    ThrowingValue(ThrowingValue&& other) noexcept(false)
        : value(other.value) {
        ++Alive;
    }

    ThrowingValue& operator=(const ThrowingValue&) = default;
    ThrowingValue& operator=(ThrowingValue&&) = default;

    ~ThrowingValue() {
        --Alive;
    }

    static void Reset() {
        Alive = 0;
        CopiesBeforeThrow = -1;
    }
};

TEST(CircularBufferExceptionTest, failedExpansionPreservesOriginalBuffer) {
    ThrowingValue::Reset();

    {
        circular_buffer<ThrowingValue, true> cb(3);
        cb.push_back(ThrowingValue(1));
        cb.push_back(ThrowingValue(2));
        cb.push_back(ThrowingValue(3));
        ThrowingValue new_value(4);

        ThrowingValue::CopiesBeforeThrow = 1;

        ASSERT_THROW(cb.push_back(new_value), std::runtime_error);
        ASSERT_EQ(cb.size(), 3);
        ASSERT_EQ(cb[0].value, 1);
        ASSERT_EQ(cb[1].value, 2);
        ASSERT_EQ(cb[2].value, 3);
        ASSERT_EQ(ThrowingValue::Alive, 4);
    }

    ASSERT_EQ(ThrowingValue::Alive, 0);
}


