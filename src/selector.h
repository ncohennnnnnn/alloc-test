/* -------------------------------------------------------------------------------
 * Copyright (c) 2018, OLogN Technologies AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -------------------------------------------------------------------------------
 *
 * Memory allocator tester -- selector
 *
 * v.1.00    Jun-22-2018    Initial release
 *
 * -------------------------------------------------------------------------------*/


#ifndef SELECTOR_H
#define SELECTOR_H

// TODO:
// (1) #include "my_allocator.h"
// (2) define MyAllocatorT properly
// (3) make sure other inclusions and/or definitions are removed or commented out :)

//#include "iib_allocator.h"
//typedef IibmallocAllocatorForTest MyAllocatorT;

#if ALLOC_TEST_HWMALLOC

#include "hw_allocator.h"
using MyAllocatorT = hwmallocAllocatorForTest<uint8_t>;

#elif ALLOC_TEST_TFMALLOC

#include "tf_allocator.h"
typedef tfmallocAllocatorForTest<uint8_t> MyAllocatorT; // change the name of the class

#else

#include "new_delete_allocator.h"
typedef NewDeleteAllocatorForTest MyAllocatorT;

#endif

#endif // SELECTOR_H
