#ifndef GC_ARAMMGR_H
#define GC_ARAMMGR_H

#include "global.h"
#include <dolphin/os.h>

#define ARAM_GRANULARITY 32
#define INVALID_ARAM_ID  0
#define ARAM_WORK_SIZE 8192


typedef struct aram_node_t
{
	struct aram_node_t *next;
	struct aram_node_t *prev;

	uint32 addr;
	uint32 size;
} aram_node_t;

class aram_id_t
{
	public:
		uint32 id;
		aram_id_t()
		{
			id = INVALID_ARAM_ID;
		}
		
		aram_id_t(aram_id_t &_id)
		{
			id = _id.id;
		}

		aram_id_t(uint32 i)
		{
			id = i;
		}

		bool operator==(aram_id_t &id2)
		{
			if( id == id2.id )
				return true;
			return false;
		}

		bool operator==(uint32 i)
		{
			if( id == i )
				return true;
			return false;
		}

		aram_id_t& operator=(aram_id_t &id2)
		{
			id = id2.id;
			return *this;
		}

		aram_id_t& operator=(uint32 i)
		{
			id = i;
			return *this;
		}
		operator uint32() { return id; }
};

class os_file;

class aram_mgr
{
	friend void aram_callback(u32);
	public:
		static void destroy()
		{
			if( initialized == true )
			{
				free( node_pool );
				free( workarea );
			}
			initialized = false;
		}

		static bool initialize(uint32 base, uint32 size, int max_allocs);
		
		static aram_id_t allocate(uint32 size);
		static void deallocate(aram_id_t id);
		
		static bool coalesce();

		static bool aram_write(aram_id_t id, uint32 offset, void *src, uint32 size);
		static bool aram_read(aram_id_t id, uint32 offset, void *dest, uint32 size);
    static bool aram_write(aram_id_t id, uint32 offset, os_file *fp, uint32 size);
   
    static bool aram_read_async(aram_id_t id, uint32 offset, void *dest, uint32 size);

    static bool aram_sync();

		static uint32 get_allocation_size( aram_id_t id )
		{
			aram_node_t *node = id_to_node( id );
      return node->size;
		}
		
		static aram_id_t address_to_id( u32 aram_addr );

		static u32 id_to_address( aram_id_t id )
		{
			assert( is_valid_id( id ) );
			return id_to_node( id )->addr;
		}
		
		static aram_id_t node_to_id(aram_node_t *node)
		{
			assert( is_valid_node( node ) );
			return( aram_id_t( (uint32)( node - node_pool ) + 1 ) );
		}
		
		static aram_node_t *id_to_node(aram_id_t id)
		{
			assert( is_valid_id( id ) );
			return( &node_pool[ id - 1 ] );
		}

		static bool is_valid_node(aram_node_t *node)
		{
			if( node >= node_pool
					&& node < ( node_pool + max_allocations ) )
				return true;
			return false;
		}

		static bool is_valid_id(aram_id_t id)
		{
			if( id > INVALID_ARAM_ID
					&& id <= max_allocations )
				return true;
			return false;
		}
		
	private:
		// Debugging stuff
		static bool is_unused(aram_node_t *node);
		static bool is_free(aram_node_t *node);
		static bool is_alloced(aram_node_t *node);
		
		// Get an unused node from the pool
		static aram_node_t *get_unused_node();
		
		// Return an unused node to the pool (used?)
		static void return_unused_node(aram_node_t *node);

		// Fork a node on the free list into two seperate nodes
		// 'node' will be changed to size 'size' and the new node
		// will be added to the free list with size node->size - 'size'
		static void fork_free(aram_node_t *node, uint32 size);
		
		// Head of list of allocated nodes
		static aram_node_t alloced_head;
		// Head of list of free nodes
		static aram_node_t free_head;
		// Head of list of unused nodes
		static aram_node_t unused_head;

		// Linearly allocated nodes
		static aram_node_t *node_pool;
		
		static uint32 heap_base;
		static uint32 heap_size;
		static uint32 max_allocations;
		static bool initialized;

		static uint8 *workarea;
		static volatile bool transfer_done;
		static volatile bool async_pending;
		static ARQRequest arq_request;
};

template< class T >	
class aram_array
{
	private:
		enum BufferNames
		{
			BUF_LOW = 0,
			BUF_MID = 1,
			BUF_HI = 2
		};
		
		template< class U >
		class buffer_node_t
		{
			public:
				int offset;
				U *ptr;
				bool dirty;
		};
		
		buffer_node_t<T> buffer_node[3];
		T *buffer;
		int objs_per_buf;
		int num_objs;
		int buffer_size;
	  aram_id_t aram_id;
		bool self_allocated_node;
		bool initialized;
	public:
		aram_array()
		{
			aram_id = 0;
			buffer = NULL;
			initialized = false;
		}
		
		aram_array(aram_id_t id, int objects_per_buffer = 0)
		{
			init( id, objects_per_buffer );
		}

		aram_array(int num_objects, int objects_per_buffer = 0)
		{
			init( num_objects, objects_per_buffer );
		}
		
		void init(aram_id_t id, int objects_per_buffer = 0)
		{
			assert( !initialized );
			assert( aram_mgr::is_valid_id( id ) );
			aram_id = id;
			num_objs = aram_mgr::get_allocation_size( aram_id ) / sizeof( T );
			init_buffers( objects_per_buffer );
			initialized = true;
		}
		
		void init(int num_objects, int objects_per_buffer = 0)
		{
			assert( !initialized );
			assert( num_objects > 0 );

			num_objs = num_objects;
			aram_id = aram_mgr::allocate( num_objs * sizeof( T ) );
			init_buffers( objects_per_buffer );
			initialized = true;
			self_allocated_node = true;
		}


		bool is_initialized()
		{
			return initialized;
		}

		void destroy()
		{
			if( initialized )
			{
				write_back_all();
				if( self_allocated_node )
					aram_mgr::deallocate( aram_id );
				free( buffer );
			}
			initialized = false;
		}

		~aram_array()
		{
			destroy();
		}

		T& operator[](int n)
		{
			assert( initialized );
			//assert( n >= 0 );
			//assert( n < num_objs );
			
			if( n < buffer_node[BUF_LOW].offset
				|| n >= ( buffer_node[BUF_HI].offset + objs_per_buf ) )
			{
				center_on( n );
				T *ptr = buffer_node[BUF_MID].ptr;
				buffer_node[BUF_MID].dirty = true;
				return *( ptr + ( n - buffer_node[BUF_MID].offset ) );
			}

			int base_offs = n - buffer_node[BUF_LOW].offset;
			buffer_node_t<T> *node = &buffer_node[ base_offs / objs_per_buf ];
			node->dirty = true;
			return *( node->ptr + ( n - node->offset ) );	
		}

		void write_back_all()
		{
			write_back( BUF_LOW );
			write_back( BUF_MID );
			write_back( BUF_HI );
		}
		
		void read_in_all()
		{
			read_in( BUF_LOW );
			read_in( BUF_MID );
			read_in( BUF_HI );
		}

		aram_id_t get_aram_id()
		{
			return aram_id;
		}

	private:
		void center_on(int n)
		{
			if( n < ( buffer_node[BUF_LOW].offset - objs_per_buf )
				|| n >= ( buffer_node[BUF_HI].offset + ( objs_per_buf * 2 ) ) )
			{
				write_back_all();
				int center_offset = ( n / objs_per_buf );

				buffer_node[BUF_LOW].offset = ( center_offset - 1 ) * objs_per_buf;
				buffer_node[BUF_MID].offset = buffer_node[BUF_LOW].offset + objs_per_buf;
				buffer_node[BUF_HI].offset = buffer_node[BUF_MID].offset + objs_per_buf;
				for( int i = 0; i < 3; i++ )
					buffer_node[ i ].dirty = false;
				
				read_in_all();
				return;
			}

			if( n < buffer_node[BUF_LOW].offset )
			{
				write_back( BUF_HI );
				T *ptr = buffer_node[BUF_HI].ptr;
				memcpy( &buffer_node[BUF_HI], &buffer_node[BUF_MID], sizeof( buffer_node_t<T> ) );
				memcpy( &buffer_node[BUF_MID], &buffer_node[BUF_LOW], sizeof( buffer_node_t<T> ) );
				buffer_node[BUF_LOW].ptr = ptr;
				buffer_node[BUF_LOW].offset -= objs_per_buf;
				buffer_node[BUF_LOW].dirty = false;
				read_in( BUF_LOW );
				return;
			}
			else
			{
				write_back( BUF_LOW );
				T *ptr = buffer_node[BUF_LOW].ptr;
				memcpy( &buffer_node[BUF_LOW], &buffer_node[BUF_MID], sizeof( buffer_node_t<T> ) );
				memcpy( &buffer_node[BUF_MID], &buffer_node[BUF_HI], sizeof( buffer_node_t<T> ) );
				buffer_node[BUF_HI].ptr = ptr;
				buffer_node[BUF_HI].offset += objs_per_buf;
				buffer_node[BUF_HI].dirty = false;
				read_in( BUF_HI );
				return;
			}

			assert( 0 );
		}

		void write_back(int buf)
		{
			assert( buf >= BUF_LOW && buf <= BUF_HI );
			
			if( buffer_node[buf].dirty == false ) return;
			
			uint32 node_size = aram_mgr::get_allocation_size( aram_id );
			void *ptr = buffer_node[buf].ptr;
			int offset = buffer_node[buf].offset * sizeof( T );
				
			buffer_node[buf].dirty = false;

			if( offset < 0 )
				return;
			
			int trans_size = node_size - offset;
			if( trans_size <= 0 )
				return;
			
			if( trans_size > buffer_size )
				trans_size = buffer_size;
			
			aram_mgr::aram_write( aram_id, offset, ptr, trans_size );
		}


		void read_in(int buf)
		{
			assert( buf >= BUF_LOW && buf <= BUF_HI );

			uint32 node_size = aram_mgr::get_allocation_size( aram_id );
			void *ptr = buffer_node[buf].ptr;
			int offset = buffer_node[buf].offset * sizeof( T );
				
			buffer_node[buf].dirty = false;

			if( offset < 0 )
				return;
			
			int trans_size = node_size - offset;
			if( trans_size <= 0 )
				return;
			
			if( trans_size > buffer_size )
				trans_size = buffer_size;

			aram_mgr::aram_read( aram_id, offset, ptr, trans_size );
		}


		void init_buffers(int objects_per_buffer)
		{
			if( objects_per_buffer <= 0 )
				objects_per_buffer = 1;

			while( ( objects_per_buffer * sizeof( T ) ) % 32 != 0 )
				objects_per_buffer++;
			
			int objs_mul = objects_per_buffer * 3;
			
			buffer = (T *) malloc( objs_mul * sizeof( T ) );
			assert( buffer != NULL );
			buffer_node[BUF_LOW].ptr = buffer;
			buffer_node[BUF_MID].ptr = buffer + objects_per_buffer;
			buffer_node[BUF_HI].ptr = buffer + ( objects_per_buffer * 2 );
			buffer_node[BUF_LOW].offset = 0;
			buffer_node[BUF_MID].offset = objects_per_buffer;
			buffer_node[BUF_HI].offset = objects_per_buffer * 2;
			
			objs_per_buf = objects_per_buffer;
			buffer_size = objs_per_buf * sizeof( T );
		}
};

			
				
			
			
				
				
				
				
	
		
#endif
