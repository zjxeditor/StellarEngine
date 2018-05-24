//
// Allocates blocks from a fixed range using buddy allocation method. Buddy allocation allows 
// reasonably fast allocation of arbitrary size blocks with minimal fragmentation and provides 
// efficient reuse of freed ranges. When a block is de-allocated an attempt is made to merge it 
// with it's neighbour (buddy) if it is contiguous and free. Based on reference implementation 
// by Bill Kristiansen.
//  

#pragma once




