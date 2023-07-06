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


class tfmallocAllocatorForTest : public allocator<uint8_t>
{
    using base = allocator<uint8_t>;
	ThreadTestRes* testRes;

public:
	tfmallocAllocatorForTest( ThreadTestRes* testRes_ ) { testRes = testRes_; }
	static constexpr bool isFake() { return false; }

	static constexpr const char* name() { return "tfmalloc allocator"; }

	void init(){}
    void deallocate(pointer const& ptr) {base::deallocate(ptr, sizeof(ptr)); }
	void deinit(){}

    // next calls are to get additional stats of the allocator, etc, if desired
	void doWhateverAfterSetupPhase() {}
	void doWhateverAfterMainLoopPhase() {}
	void doWhateverAfterCleanupPhase() {}

	ThreadTestRes* getTestRes() { return testRes; }

};




#endif // TFMALLOC_ALLOCATOR_H