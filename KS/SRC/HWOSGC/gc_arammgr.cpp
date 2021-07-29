#include "gc_arammgr.h"
#include "osfile.h"

#define ASSERT_VALID_ARAM_ID(id) \
	assert( is_valid_id( id ) );

#define ASSERT_VALID_ARAM_NODE(node) \
	assert( is_valid_node( node ) );

#if defined(USER_MJD)
#define ARAMMGR_VERBOSE 1
#endif //USER_MJD

#define ARAMMGR_PRIORITY ARQ_PRIORITY_HIGH

aram_node_t aram_mgr::alloced_head = { NULL, NULL, 0, 0 };
aram_node_t aram_mgr::free_head = { NULL, NULL, 0, 0 };
aram_node_t aram_mgr::unused_head = { NULL, NULL, 0, 0 };
aram_node_t *aram_mgr::node_pool = NULL;
uint32 aram_mgr::heap_base = 0;
uint32 aram_mgr::heap_size = 0;
uint32 aram_mgr::max_allocations = 0;
bool aram_mgr::initialized = false;
uint8 *aram_mgr::workarea = NULL;
volatile bool aram_mgr::transfer_done = false;
volatile bool aram_mgr::async_pending = false;
ARQRequest aram_mgr::arq_request = {};

void aram_callback(u32 request)
{
	//we don't care about anything, just flag the transfer
	aram_mgr::transfer_done = true;
  aram_mgr::async_pending = false;
}

bool aram_mgr::initialize(uint32 base, uint32 size, int max_allocs)
{
	assert( initialized == false );
	assert( ( base % ARAM_GRANULARITY ) == 0 );
	assert( ( size % ARAM_GRANULARITY ) == 0 );
	assert( max_allocs > 0 );
	
	ARInit( NULL, 0 );
	assert( ARCheckInit() );

	ARQInit();

#if defined(ARAMMGR_VERBOSE)
	OSReport( "Initializing ARAM Manager\n"
						"heap_base       = 0x%08X\n"
						"heap_size       = 0x%08X\n"
						"max_allocations = %d\n", base, size, max_allocs );
#endif //ARAMMGR_VERBOSE
	
	// Initialize member variables
	heap_base = base;
	heap_size = size;
	max_allocations = max_allocs;

	// Allocate node pool
	node_pool = (aram_node_t *) malloc( sizeof( aram_node_t ) * max_allocations );
	assert( node_pool != NULL );
	memset( node_pool, 0, sizeof( aram_node_t ) * max_allocations );

	// Fixup pointers for alloced and unused lists
	alloced_head.next = &alloced_head;
	alloced_head.prev = &alloced_head;
	
	aram_node_t *current = &node_pool[0];
	aram_node_t *end = &node_pool[ max_allocations - 1 ];
	
	unused_head.next = current;
	unused_head.prev = end;
	end->next = &unused_head;
	
	current->prev = &unused_head;
	current->next = current + 1;

	current++;
	
	while( current != end )
	{
		current->prev = current - 1;
		current->next = current + 1;
		current++;
	}

	current->prev = current - 1;
	
	// Add a free node to the free list
	// This is essentially the root node for the free list
	// Its size and address are equal to the heap parameters
	aram_node_t *new_free = get_unused_node();

	new_free->size = heap_size;
	new_free->addr = heap_base;

	free_head.next = free_head.prev = new_free;
	new_free->next = new_free->prev = &free_head;
	
	workarea = (uint8 *) memalign( 64, ARAM_WORK_SIZE );
	return true;
}

aram_node_t *aram_mgr::get_unused_node()
{
	assert( unused_head.next != &unused_head ); // Out of unused nodes

	aram_node_t *node = unused_head.next;
	unused_head.next = node->next;
	node->next->prev = &unused_head;
#if defined(ARAMMGR_VERBOSE)
	OSReport( "Getting unused node %u (0x%08X)\n", (uint32)node_to_id( node ), node );
#endif //ARAMMGR_VERBOSE
	return node;
}

void aram_mgr::return_unused_node(aram_node_t *node)
{
	ASSERT_VALID_ARAM_NODE( node );
	assert( !is_alloced( node ) );
	assert( !is_free( node ) );
	assert( !is_unused( node ) );
	
#if defined(ARAMMGR_VERBOSE)
	OSReport( "Returning unused node %u (0x%08X)\n", (uint32)node_to_id( node ), node );
#endif //ARAMMGR_VERBOSE
	
	node->next = unused_head.next;
	node->prev = &unused_head;
	node->next->prev = node;
	unused_head.next = node;
}

void aram_mgr::fork_free(aram_node_t *node, uint32 size)
{
	ASSERT_VALID_ARAM_NODE( node );
	assert( is_free( node ) );
	assert( size > 0 );
	assert( ( size % ARAM_GRANULARITY ) == 0 );
	
#if defined(ARAMMGR_VERBOSE)
	OSReport( "Forking node %u (0x%08X)\n", (uint32)node_to_id( node ), node );
#endif //ARAMMGR_VERBOSE
	
	aram_node_t *new_node = get_unused_node();
	ASSERT_VALID_ARAM_NODE( new_node );
	
	new_node->size = node->size - size;
	new_node->addr = node->addr + size;

	node->size = size;
#if defined(ARAMMGR_VERBOSE)
	OSReport( "  node->size = %u, new_node->addr = 0x%08X\n", node->size, node->addr );
	OSReport( "  new_node->size = %u, new_node->addr = 0x%08X\n", new_node->size, new_node->addr );
#endif //ARAMMGR_VERBOSE
	
	if( new_node->size > node->size )
	{
		while( new_node->size > node->size && node != &free_head )
			node = node->next;
		
		new_node->next = node;
		new_node->prev = node->prev;
		node->prev = new_node;
		new_node->prev->next = new_node;
	}
	else
	{
		while( new_node->size < node->size )
			node = node->prev;
		
		new_node->prev = node;
		new_node->next = node->next;
		node->next = new_node;
		new_node->next->prev = new_node;
	}
}

aram_id_t aram_mgr::allocate(uint32 size)
{
#if defined(ARAMMGR_VERBOSE)
	OSReport( "Trying to allocate %u bytes\n", size );
#endif //ARAMMGR_VERBOSE
	
	aram_node_t *node = free_head.next;

	if( size == 0 )
		return INVALID_ARAM_ID;

	size = OSRoundUp32B( size );
	
	while( node->size < size && node != &free_head )
		node = node->next;

	if( node == &free_head )
	{
#if defined(ARAMMGR_VERBOSE)
		OSReport( "Not enough free space\n" );
#endif //ARAMMGR_VERBOSE
		return INVALID_ARAM_ID;
	}

#if defined(ARAMMGR_VERBOSE)
	OSReport( "Using node %u (0x%08X)\n", (uint32)node_to_id( node ), node );
#endif //ARAMMGR_VERBOSE
	
	if( node->size != size )
		fork_free( node, size );

	node->next->prev = node->prev;
	node->prev->next = node->next;

	node->prev = &alloced_head;
	node->next = alloced_head.next;
	node->next->prev = node;
	node->prev->next = node;

	return( node_to_id( node ) );
}

void aram_mgr::deallocate(aram_id_t id)
{
	ASSERT_VALID_ARAM_ID( id );
	
#if defined(ARAMMGR_VERBOSE)
	OSReport( "Deallocating node %u (0x%08X)\n", (uint32)id, id_to_node( id ) );
#endif //ARAMMGR_VERBOSE
	
	aram_node_t *node = id_to_node( id );
	ASSERT_VALID_ARAM_NODE( node );
	assert( is_alloced( node ) );
	
	aram_node_t *searcher = free_head.next;
  aram_node_t *left = NULL;
  aram_node_t *right = NULL;
 
  while( searcher != &free_head )
	{
		if( ( searcher->addr + searcher->size ) == node->addr )
      left = searcher;
    else if( searcher->addr == ( node->addr + node->size ) )
      right = searcher;
 
    if( left && right )
      break;

    searcher = searcher->next;
	}

	node->prev->next = node->next;
	node->next->prev = node->prev;

  if( left || right )
  {
	  if( left )
    {
#if defined(ARAMMGR_VERBOSE)
      OSReport( "Attaching node %d to node %d\n", (uint32)node_to_id( node ), (uint32)node_to_id( left ) );
#endif
      left->prev->next = left->next;
      left->next->prev = left->prev;
      left->size += node->size;
      return_unused_node( node );
      node = left;
    }

    if( right )
    {
#if defined(ARAMMGR_VERBOSE)
      OSReport( "Attaching node %d to node %d\n", (uint32)node_to_id( right ), (uint32)node_to_id( node ) );
#endif
      right->prev->next = right->next;
      right->next->prev = right->prev;
      node->size += right->size;
      return_unused_node( right );
    }
  }
  
  searcher = free_head.next;

	while( searcher->size < node->size && searcher != &free_head )
		searcher = searcher->next;

	node->next = searcher;
	node->prev = searcher->prev;
	searcher->prev = node;
	node->prev->next = node;
}

bool aram_mgr::is_unused(aram_node_t *node)
{
	ASSERT_VALID_ARAM_NODE( node );
	aram_node_t *cur = unused_head.next;

	while( cur != &unused_head )
	{
		if( cur == node )
		{
			return true;
		}
		cur = cur->next;
	}

	return false;
}

bool aram_mgr::is_free(aram_node_t *node)
{
	ASSERT_VALID_ARAM_NODE( node );
	aram_node_t *cur = free_head.next;

	while( cur != &free_head )
	{
		if( cur == node )
		{
			return true;
		}
		cur = cur->next;
	}

	return false;
}

bool aram_mgr::is_alloced(aram_node_t *node)
{
	ASSERT_VALID_ARAM_NODE( node );
	aram_node_t *cur = alloced_head.next;

	while( cur != &alloced_head )
	{
		if( cur == node )
		{
			return true;
		}
		cur = cur->next;
	}

	return false;
}

aram_id_t aram_mgr::address_to_id( u32 aram_addr )
{
	aram_node_t *cur = alloced_head.next;

	while( cur != &alloced_head )
	{
		if( cur->addr == aram_addr )
		{
			return node_to_id( cur );
		}
		cur = cur->next;
	}

	return INVALID_ARAM_ID;
}		

bool aram_mgr::aram_write(aram_id_t id, uint32 offset, void *_src, uint32 size)
{
	uint8 *src = (uint8 *)_src;
	
	ASSERT_VALID_ARAM_ID( id );
	assert( src != NULL );
	assert( ( offset & 0x0000001f ) == 0 ); // Everything must be aligned to 4 bytes
	assert( size > 0 );
	
	size = OSRoundUp32B( size );
	
	aram_node_t *node = id_to_node( id );
	
	assert( offset + size <= node->size );
	
	aram_sync();
  while( size != 0 )
	{
		uint32 trans_size = size > ARAM_WORK_SIZE ? ARAM_WORK_SIZE : size;
		transfer_done = 0;
		memcpy( workarea, src, trans_size );
		DCFlushRange( workarea, trans_size );
		ARQPostRequest( &arq_request, 0, ARQ_TYPE_MRAM_TO_ARAM, ARAMMGR_PRIORITY,
				(uint32) workarea, node->addr + offset, trans_size, aram_callback );
		while( transfer_done == 0 ) {};
		size -= trans_size;
		src += trans_size;
		offset += trans_size;
	}
	
	return true;
}

bool aram_mgr::aram_read(aram_id_t id, uint32 offset, void *_dest, uint32 size)
{
	uint8 *dest = (uint8 *)_dest;
	
	ASSERT_VALID_ARAM_ID( id );
	assert( dest != NULL );
	assert( ( offset & 0x0000001f ) == 0 ); // Everything must be aligned to 4 bytes
	assert( size > 0 );
	
	size = OSRoundUp32B( size );
	
	aram_node_t *node = id_to_node( id );
	
	assert( offset + size <= node->size );
	
  aram_sync();

	while( size != 0 )
	{
		uint32 trans_size = size > ARAM_WORK_SIZE ? ARAM_WORK_SIZE : size;
		transfer_done = 0;
		DCInvalidateRange( workarea, trans_size );
		ARQPostRequest( &arq_request, 0, ARQ_TYPE_ARAM_TO_MRAM, ARAMMGR_PRIORITY,
				node->addr + offset, (uint32) workarea, trans_size, aram_callback );
		while( transfer_done == 0 ) {};
		memcpy( dest, workarea, trans_size );
		size -= trans_size;
		dest += trans_size;
		offset += trans_size;
	}
	
	return true;
}

bool aram_mgr::aram_write(aram_id_t id, uint32 offset, os_file *fp, uint32 size)
{
	ASSERT_VALID_ARAM_ID( id );
	assert( fp != NULL );
  assert( fp->is_open() );
	assert( ( offset & 0x0000001f ) == 0 ); // Everything must be aligned to 4 bytes
	assert( size > 0 );

  assert( ( fp->get_size() - fp->get_fp() ) >= size );
  
	size = OSRoundUp32B( size );
	
	aram_node_t *node = id_to_node( id );
	
	assert( offset + size <= node->size );
	
  aram_sync();
  
	while( size != 0 )
	{
		uint32 trans_size = size > ARAM_WORK_SIZE ? ARAM_WORK_SIZE : size;
		transfer_done = 0;
		fp->read( workarea, trans_size, false );
		DCFlushRange( workarea, trans_size );
		ARQPostRequest( &arq_request, 0, ARQ_TYPE_MRAM_TO_ARAM, ARAMMGR_PRIORITY,
				(uint32) workarea, node->addr + offset, trans_size, aram_callback );
		while( transfer_done == 0 ) {};
		size -= trans_size;
		offset += trans_size;
	}
	
	return true;
}

bool aram_mgr::aram_read_async(aram_id_t id, uint32 offset, void *_dest, uint32 size)
{
	uint8 *dest = (uint8 *)_dest;
	
	ASSERT_VALID_ARAM_ID( id );
	assert( dest != NULL );
  assert( ( (unsigned int)dest & 0x0000001f ) == 0 );
	assert( ( offset & 0x0000001f ) == 0 ); // Everything must be aligned to 32 bytes
	assert( size > 0 );
	assert( ( size & 0x0000001f ) == 0 );
  
	aram_node_t *node = id_to_node( id );
	
	assert( offset + size <= node->size );
	
  aram_sync();

	transfer_done = 0;
  async_pending = true;
  DCInvalidateRange( dest, size );
  ARQPostRequest( &arq_request, 0, ARQ_TYPE_ARAM_TO_MRAM, ARAMMGR_PRIORITY,
			node->addr + offset, (uint32) dest, size, aram_callback );
	return true;
}

bool aram_mgr::aram_sync()
{
  if( async_pending )
  {
    while( transfer_done == 0 ) {};
    async_pending = false;
    return true;
  }
  return false;
}


