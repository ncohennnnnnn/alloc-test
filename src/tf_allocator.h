/* -------------------------------------------------------------------------------
 * Added file     Jul-04-2023
 * -------------------------------------------------------------------------------
 *
 * Memory allocator tester -- tfmalloc
 *
 * -------------------------------------------------------------------------------*/


#ifndef TFMALLOC_ALLOCATOR_H
#define TFMALLOC_ALLOCATOR_H

#include "test_common.h"
#include "simple_void_ptr.h"


template<typename T>
class tfmallocAllocatorForTest : public allocator<T>
{
    using base = allocator<T>;
    using pointer = fancy_ptr<T*>;
    ThreadTestRes* testRes;

public:
    template<typename U>
    struct other_alloc
    {
        using other = tfmallocAllocatorForTest<U>;
    };
    template<typename U>
    using rebind = other_alloc<U>;

public:
    tfmallocAllocatorForTest( ThreadTestRes* testRes_ ) { testRes = testRes_; }
    using is_fake = std::false_type;
    using is_fancy = std::true_type;

    static constexpr bool isFake() { return false; }
    static constexpr bool isFancy() { return true; }

    static constexpr const char* name() { return "tfmalloc allocator"; }

    void init(){}
    void deallocate(pointer ptr) {base::deallocate(ptr); } // got rid of the const&
    void deinit(){}

    // next calls are to get additional stats of the allocator, etc, if desired
    void doWhateverAfterSetupPhase() {}
    void doWhateverAfterMainLoopPhase() {}
    void doWhateverAfterCleanupPhase() {}

    ThreadTestRes* getTestRes() { return testRes; }

};




#endif // TFMALLOC_ALLOCATOR_H
