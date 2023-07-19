/* -------------------------------------------------------------------------------
 * Added file     Jul-04-2023
 * -------------------------------------------------------------------------------
 * 
 * Memory allocator tester -- void allocator for hwmalloc (used for estimating cost of test itself)
 *  
 * -------------------------------------------------------------------------------*/

 
#ifndef VOID_FPTR_ALLOCATOR_H
#define VOID_FPTR_ALLOCATOR_H

#include "test_common.h"

#if ALLOC_TEST_HWMALLOC

#include "hw_allocator.h"
using ActualAllocator = hwmallocAllocatorForTest<uint8_t>;

#elif ALLOC_TEST_TFMALLOC

#include "tf_allocator.h"
using ActualAllocator = tfmallocAllocatorForTest<uint8_t>;

#endif



template<class ActualAllocator>
class VoidAllocatorForTest
{
    ThreadTestRes* testRes;
    ThreadTestRes discardedTestRes;
    ActualAllocator alloc;
    using pointer = decltype(alloc.allocate(1));
    pointer fakeBuffer = nullptr;
    static constexpr size_t fakeBufferSize = 0x1000000;

public:
    template<typename U>
    struct other_alloc
    {
        using new_alloc = typename ActualAllocator::template other_alloc<U>::other;
        using other = VoidAllocatorForTest<new_alloc>;
    };

    template<typename U>
    using rebind = other_alloc<U>;

public:
    VoidAllocatorForTest( ThreadTestRes* testRes_ ) : alloc( &discardedTestRes ) { testRes = testRes_; }

    using is_fake = std::true_type;
    using is_fancy = std::true_type;
    static constexpr bool isFake() { return true; } // thus indicating that certain checks over allocated memory should be ommited
    static constexpr bool isFancy() { return true; }

    #if ALLOC_TEST_HWMALLOC
    static constexpr const char* name() { return "void hw allocator"; }
    #elif ALLOC_TEST_TFMALLOC
    static constexpr const char* name() { return "void tf allocator"; }
    #else
    static constexpr const char* name() { return "void allocator"; }
    #endif

    void init()
    {
        alloc.init();
        fakeBuffer = alloc.allocate( fakeBufferSize );
    }
    auto allocateSlots( size_t sz ) { static_assert( isFake()); assert( sz <= fakeBufferSize ); return alloc.allocate( sz ); }
    auto allocate( size_t sz ) { assert( sz <= fakeBufferSize ); return fakeBuffer; }
    void deallocate( pointer ptr ) {} // maybe try with const&
    void deallocateSlots( pointer ptr ) {alloc.deallocate( ptr );} // maybe try with const&
    void deinit() { if ( fakeBuffer ) alloc.deallocate( fakeBuffer ); fakeBuffer = nullptr; }

    // next calls are to get additional stats of the allocator, etc, if desired
    void doWhateverAfterSetupPhase() {}
    void doWhateverAfterMainLoopPhase() {}
    void doWhateverAfterCleanupPhase() {}

    ThreadTestRes* getTestRes() { return testRes; }
};




#endif // VOID_FPTR_ALLOCATOR_H