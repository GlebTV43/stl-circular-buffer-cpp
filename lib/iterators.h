#pragma once

#include "circular_buffer.h" // pragma once остановит тк я подключаю его в самом конце. чисто чтобы эту бесячую красную полоску убрать 

#include <iterator>
#include <memory>
#include <type_traits>

template<typename T, bool Extendable, typename Allocator>
template<bool IsConst, bool IsReverse> 
class circular_buffer<T, Extendable, Allocator>::basic_iterator {
public:
    using container_type = circular_buffer<T, Extendable, Allocatr>;

    using iterator_category = std::random_access_iterator_tag;

    using value_type = T;

    using difference_type = typename container_type::difference_type;
    
    using reference = std::conditional_t<IsConst, const value_type&, value_type&>;

    using reference = std::conditional_t<IsConst, const value_type*, value_type*>;  
};