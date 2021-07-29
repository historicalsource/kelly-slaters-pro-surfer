// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include <ctype.h>

#include "stringx.h"


// define this to enable more aggressive caching, at the cost of
// a small amount of performance
// this is actually very effective; the cache is heavily used
#define FASCIST_CACHING


#if !defined(BUILD_BOOTABLE)
#define STRINGX_DEBUG
#endif

#ifdef STRINGX_DEBUG
// set debugger watches on these to monitor string use
// peak = # used, not # free
unsigned int peak_small_strings, peak_medium_strings, peak_long_strings;
unsigned int cache_hits, cache_misses;
#endif

// increase these numbers if you run out of strings
// try to increase only the category that ran out
#define STRINGX_MAX_SHORT_STRINGS   15000
#define STRINGX_MAX_MEDIUM_STRINGS  7000
//#if defined(TARGET_XBOX)
#define STRINGX_MAX_LONG_STRINGS    100
//#else
//#define STRINGX_MAX_LONG_STRINGS    50
//#endif /* TARGET_XBOX JIV DEBUG */
//125

//#define STRINGX_MAX_SHORT_STRINGS   9500
//#define STRINGX_MAX_MEDIUM_STRINGS  4000
//#define STRINGX_MAX_LONG_STRINGS    100

#define STRINGX_TOTAL_STRINGS       (STRINGX_MAX_SHORT_STRINGS + STRINGX_MAX_MEDIUM_STRINGS + STRINGX_MAX_LONG_STRINGS)

#define STRINGX_SHORT_STRING_MAX_LENGTH    32
#define STRINGX_MEDIUM_STRING_MAX_LENGTH   80
#define STRINGX_LONG_STRING_MAX_LENGTH    1024


#define STRING_POOL_BLOCKS ((STRINGX_MAX_SHORT_STRINGS * STRINGX_SHORT_STRING_MAX_LENGTH + \
	STRINGX_MAX_MEDIUM_STRINGS * STRINGX_MEDIUM_STRING_MAX_LENGTH + \
STRINGX_MAX_LONG_STRINGS * STRINGX_LONG_STRING_MAX_LENGTH) / sizeof (big_int))

bool stringx::stringx_initialized = false;

string_buf stringx::strings[STRINGX_TOTAL_STRINGS];
big_int stringx::string_pool[STRING_POOL_BLOCKS];
int stringx::npos = -1;

string_buf *stringx::free_small_buffers[STRINGX_MAX_SHORT_STRINGS];   unsigned int stringx::free_small_buffers_end;
string_buf *stringx::free_medium_buffers[STRINGX_MAX_MEDIUM_STRINGS]; unsigned int stringx::free_medium_buffers_end;
string_buf *stringx::free_long_buffers[STRINGX_MAX_LONG_STRINGS];    unsigned int stringx::free_long_buffers_end;

// number of cache buckets per starting character
// the cache hashing function just takes the first character of the string
// this is not a very good hash function, but it's fast and seems to work
// well enough
#define STRINGX_CACHE_BUCKETS       16

string_buf *stringx::buf_cache[256 * STRINGX_CACHE_BUCKETS];     unsigned int stringx::buf_cache_lru[256];

stringx empty_string = stringx("");
stringx sendl = stringx("\n");



void string_buf::clear()
{
	assert(ref_count == 0 && "Attempted to clear string_buf with active references");
	assert(data);
	memset(data, 0, max_blocks * sizeof (big_int));
}


void string_buf::null_terminate() const
{
	char *chars = reinterpret_cast<char *>(data);
	if (char_length > get_char_capacity()) {
		error("Char length: %i  Char capacity: %i  Why, oh why?", char_length, get_char_capacity());
	}
	chars[char_length] = '\0';
}


stringx::stringx()
{
	if (!stringx_initialized) stringx::init();
	my_buf = acquire_buffer("", -1);
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx::stringx(const char *str, int len)
{
	if (!stringx_initialized) stringx::init();
	char *s = const_cast<char *>(str);  // we don't actually modify it
	if (s == NULL) s = "";
	assert(s != NULL);
	if (len == -1) len = strlen(s);
	my_buf = acquire_buffer(str, len);
	assert(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx::stringx(const stringx &cp)
{
	if (!stringx_initialized) stringx::init();
	my_buf = cp.my_buf;
	my_buf->inc_ref();
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx::stringx(float f)
{
	if (!stringx_initialized) stringx::init();
	char work[128];
	int len = sprintf(work,"%0.3f",f);
	my_buf = acquire_buffer(work, len);
	assert(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx::stringx(int i)
{
	if (!stringx_initialized) stringx::init();
	char work[128];
	int len = sprintf(work,"%i",i);
	
	my_buf = acquire_buffer(work, len);
	assert(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx::stringx(unsigned int i)
{
	if (!stringx_initialized) stringx::init();
	char work[128];
	int len = sprintf(work,"%u",i);
	
	my_buf = acquire_buffer(work, len);
	assert(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx::stringx(stringx::fmtd, const char *fmtp, ...)
{
	if (!stringx_initialized) stringx::init();
	va_list vlist;
	va_start(vlist, fmtp);
	char fmtbuff[1024];
	int len = vsprintf(fmtbuff, fmtp, vlist);
	my_buf = acquire_buffer(fmtbuff, len);
	assert(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx::~stringx()
{
	assert(my_buf);
	release_buffer();
}


bool stringx::is_buffer_mine(string_buf *buf) const
{
	if (buf >= &strings[0] && buf < &strings[STRINGX_TOTAL_STRINGS])
		return true;
	else
		return false;
}


void stringx::release_buffer()
{
	assert(my_buf);
	assert(my_buf->ref_count > 0);
	if ( my_buf && my_buf->ref_count > 0 )
	{
		my_buf->dec_ref();
		
		if (my_buf->get_ref() > 0 || !is_buffer_mine(my_buf)) {
			// don't add buffers we don't own to the free list
			my_buf = NULL;
			chars = NULL;
			return;
		}
		
		assert(my_buf->get_ref() == 0);
		
		// Hacky fun pointer math to figure out which size class we're in
		// If there are no more references to this buffer, add it to the appropriate free list
		unsigned int slot = (my_buf - &strings[0]);
		assert(slot < STRINGX_TOTAL_STRINGS);
		unsigned int *end;
		string_buf **free_list;
		
		if (slot < STRINGX_MAX_SHORT_STRINGS) {
			free_list = free_small_buffers;
			end = &free_small_buffers_end;
		} else if (slot < STRINGX_MAX_MEDIUM_STRINGS + STRINGX_MAX_SHORT_STRINGS) {
			free_list = free_medium_buffers;
			end = &free_medium_buffers_end;
		} else {
			free_list = free_long_buffers;
			end = &free_long_buffers_end;
		}
		free_list[*end] = my_buf;
		(*end)++;
		// else the buffer came from somewhere else
		
		my_buf = NULL;
	}
}


void stringx::init()
{
	int i, s;
	big_int *buf_pos = string_pool;
	
	for (i = 0, s = 0; i < STRINGX_MAX_SHORT_STRINGS; i++, s++, buf_pos += STRINGX_SHORT_STRING_MAX_LENGTH / sizeof (big_int)) {
		strings[i].set_buf(buf_pos, STRINGX_SHORT_STRING_MAX_LENGTH / sizeof (big_int));
		free_small_buffers[s] = &strings[i];
		assert(strings[i].get_ref() == 0);
	}
	free_small_buffers_end = s;
	
	for (s = 0; i < STRINGX_MAX_SHORT_STRINGS + STRINGX_MAX_MEDIUM_STRINGS; i++, s++, buf_pos += STRINGX_MEDIUM_STRING_MAX_LENGTH / sizeof (big_int)) {
		strings[i].set_buf(buf_pos, STRINGX_MEDIUM_STRING_MAX_LENGTH / sizeof (big_int));
		free_medium_buffers[s] = &strings[i];
		assert(strings[i].get_ref() == 0);
	}
	free_medium_buffers_end = s;
	
	for (s = 0; i < STRINGX_TOTAL_STRINGS; i++, s++, buf_pos += STRINGX_LONG_STRING_MAX_LENGTH / sizeof (big_int)) {
		strings[i].set_buf(buf_pos, STRINGX_LONG_STRING_MAX_LENGTH / sizeof (big_int));
		free_long_buffers[s] = &strings[i];
		assert(strings[i].get_ref() == 0);
	}
	free_long_buffers_end = s;
	
	assert(i == STRINGX_TOTAL_STRINGS);
	assert(s == STRINGX_MAX_LONG_STRINGS);
	assert((buf_pos - string_pool) == STRING_POOL_BLOCKS);
	
	for (i = 0; i < 256 * STRINGX_CACHE_BUCKETS; i++) {
		buf_cache[i] = NULL;
	}
	
	for (i = 0; i < 256; i++) {
		buf_cache_lru[i] = i * STRINGX_CACHE_BUCKETS;
	}
	
#ifdef STRINGX_DEBUG
	peak_small_strings = 0;
	peak_medium_strings = 0;
	peak_long_strings = 0;
	cache_hits = 0;
	cache_misses = 0;
#endif
	
	stringx_initialized = true;
}


string_buf *stringx::acquire_buffer(const char *str, int len)
{
	string_buf *buf;
	
	buf = find_cached_string(str);
	if (buf == NULL) {
#ifdef STRINGX_DEBUG
		cache_misses++;
#endif
		if (len == -1)
			len = strlen(str);
		buf = find_empty_buffer(len, str);
		assert(buf);
		buf->set_to_cstr(str, len);
		add_buf_to_cache(buf);
	} else {
#ifdef STRINGX_DEBUG
		cache_hits++;
#endif
	}
	
	return buf;
}


string_buf *stringx::find_small_buffer()
{
	string_buf *buf = NULL;
	
	do {
		if (free_small_buffers_end == 0) {
			assert(false && "Optimization: please add more small string buffers (can ignore this assert).");
			return find_medium_buffer();
		}
		
		free_small_buffers_end--;
		buf = free_small_buffers[free_small_buffers_end];
	} while (buf == NULL || buf->get_ref() > 0);
	
	return buf;
}


string_buf *stringx::find_medium_buffer()
{
	string_buf *buf = NULL;
	
	do {
		if (free_medium_buffers_end == 0) {
			assert(false && "Optimization: please add more medium string buffers (can ignore this assert).");
			return find_large_buffer();
		}
		
		free_medium_buffers_end--;
		buf = free_medium_buffers[free_medium_buffers_end];
	} while (buf == NULL || buf->get_ref() > 0);
	
	return buf;
}


string_buf *stringx::find_large_buffer()
{
	string_buf *buf = NULL;
	
	do {
		if (free_long_buffers_end == 0) {
			error("Out of large string buffers.");
			return NULL;
		}
		
		free_long_buffers_end--;
		buf = free_long_buffers[free_long_buffers_end];
	} while (buf == NULL || buf->get_ref() > 0);
	
	return buf;
	
}


string_buf *stringx::find_empty_buffer( int capacity, const char* str_just_for_error_msgs )
{
	string_buf *tmpbuf = NULL;
	
	if (capacity < STRINGX_SHORT_STRING_MAX_LENGTH-1)
		tmpbuf = find_small_buffer();
	else if (capacity < STRINGX_MEDIUM_STRING_MAX_LENGTH-1)
		tmpbuf = find_medium_buffer();
	else if (capacity < STRINGX_LONG_STRING_MAX_LENGTH-1)
		tmpbuf = find_large_buffer();
	else
		// Lasciate ogne speranza, voi ch'intrate!
		error("String [%s] too long. Bad, bad, bad.", str_just_for_error_msgs );
	
	assert(tmpbuf);
	assert(tmpbuf->get_ref() == 0);
	assert(tmpbuf->data != NULL);
	
	tmpbuf->clear();
	
#ifdef STRINGX_DEBUG
	int cur_small_strings = STRINGX_MAX_SHORT_STRINGS - free_small_buffers_end;
	int cur_medium_strings = STRINGX_MAX_MEDIUM_STRINGS - free_medium_buffers_end;
	int cur_long_strings = STRINGX_MAX_LONG_STRINGS - free_long_buffers_end;
	if (peak_small_strings < (unsigned)cur_small_strings) peak_small_strings = cur_small_strings;
	if (peak_medium_strings < (unsigned)cur_medium_strings) peak_medium_strings = cur_medium_strings;
	if (peak_long_strings < (unsigned)cur_long_strings) peak_long_strings = cur_long_strings;
#endif
	
	tmpbuf->inc_ref();
	assert(tmpbuf->get_ref() == 1);
	
	return tmpbuf;
}


string_buf *stringx::find_cached_string(const char *str, int len)
{
	// Uncomment this if you suspect a caching problem. This will disable caching.
	// #pragma fixme("String caching disabled. Please re-enable it as soon as possible.")
	// return NULL;
	
	if (str == NULL) {
		return NULL;
	}
	
	if (len == -1)
		len = strlen(str);
	
	unsigned int start = ((unsigned char) str[0]) * STRINGX_CACHE_BUCKETS;
	
	for (unsigned int i = start; i < start + STRINGX_CACHE_BUCKETS; i++) {
		if (buf_cache[i] != NULL) {
			if (buf_cache[i]->get_ref() > 0 && buf_cache[i]->is_equal(str) && buf_cache[i]->char_length == len) {
				buf_cache[i]->inc_ref();
				return buf_cache[i];
			}
		}
	}
	
	return NULL;
}


void stringx::add_buf_to_cache(string_buf *buf)
{
	assert(buf != NULL);
	assert(buf->data != NULL);
	
	if (buf >= &strings[0] && buf < &strings[STRINGX_TOTAL_STRINGS]) {
		unsigned char first = reinterpret_cast<char *>(buf->data)[0];
		unsigned int start = (unsigned int)((unsigned char *)buf->data)[0] * STRINGX_CACHE_BUCKETS;
		for (unsigned int i = start; i < start+STRINGX_CACHE_BUCKETS; i++) {
			if (buf_cache[i] == buf) {
				return;
			}
		}
		buf_cache[buf_cache_lru[first]] = buf;
		
		if ((buf_cache_lru[first] & 0xFF) == 0xFF) {
			buf_cache_lru[first] -= 0xFF;
		} else {
			buf_cache_lru[first]++;
		}
	} // else we don't own it
}


void stringx::fork_data(int new_len)
{
	assert(my_buf);
	
	if (new_len == -1)
		new_len = my_buf->char_length;
	
	// this should not happen often
	// it causes a bit of excessive copying
	if (new_len < my_buf->char_length)
		truncate(new_len);
	
	string_buf *buf = find_empty_buffer(new_len, chars);
	assert(buf != NULL);
	buf->copy_data_from(my_buf);
	buf->null_terminate();
	release_buffer();
	my_buf = buf;
	chars = reinterpret_cast<char *>(my_buf->data);
}


void stringx::truncate(int new_len)
{
	assert(my_buf);
	assert(new_len <= my_buf->char_length);
	assert(new_len >= 0);
	
	// if someone else is sharing this buffer, we need to fork it
	lock();
	assert(my_buf->get_ref() == 1);
	
	my_buf->set_char_length(new_len);
	chars = reinterpret_cast<char *>(my_buf->data);
}


stringx &stringx::operator=(const stringx &cp)
{
	release_buffer();
	my_buf = cp.my_buf;
	my_buf->inc_ref();
	add_buf_to_cache(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);
	
	return *this;
}


stringx &stringx::operator=(const char *str)
{
	copy(str);
	return *this;
}


stringx &stringx::operator+=(const stringx &cp)
{
	append(cp);
	return *this;
}


stringx &stringx::operator+=(const char *str)
{
	append(str);
	return *this;
}


bool stringx::aggressively_cache_buffer()
{
	string_buf *tmp_buf = find_cached_string(chars, my_buf->char_length);
	if (tmp_buf != NULL && tmp_buf != my_buf) {
		release_buffer();
		my_buf = tmp_buf;
		chars = reinterpret_cast<char *>(my_buf->data);
		return true;
	} else if (tmp_buf == my_buf) {
		tmp_buf->dec_ref();
	}
	return false;
}


void stringx::make_room(int size)
{
	assert(my_buf);
	if (my_buf->get_char_capacity() < size) {
		fork_data(size);
	}
	chars = reinterpret_cast<char *>(my_buf->data);
}


void stringx::copy(const char *str, int len)
{
	if (str == NULL) {
		copy("");
		return;
	}
	assert(str != NULL);
	if (len == -1) len = strlen(str);
	
	release_buffer();
	
	my_buf = acquire_buffer(str, len);
	assert(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);
}


void stringx::copy(stringx &cp)
{
	release_buffer();
	my_buf = cp.my_buf;
	my_buf->inc_ref();
	add_buf_to_cache(my_buf);
	
	chars = reinterpret_cast<char *>(my_buf->data);
}


int stringx::printf(const char *fmtp, ...)
{
	va_list vlist;
	va_start(vlist, fmtp);
	release_buffer();

	char fmtbuff[1024];
	int len = vsprintf(fmtbuff, fmtp, vlist);
	my_buf = acquire_buffer(fmtbuff, len);
	assert(my_buf);
	chars = reinterpret_cast<char *>(my_buf->data);

	return len;
}


void stringx::append(const char *str, int len)
{
	if (str == NULL) {
		append("");
		return;
	}
	if (len == -1) len = strlen(str);
	lock();
	make_room(my_buf->char_length + len);
	memcpy(chars + my_buf->char_length, str, len);
	bool cache_it = false;
	if (my_buf->char_length == 0)
		cache_it = true;
	my_buf->set_char_length(my_buf->char_length + len);
	my_buf->null_terminate();
#ifdef FASCIST_CACHING
	if (!aggressively_cache_buffer())
		// nested
#endif
		if (cache_it)
			add_buf_to_cache(my_buf);
}


void stringx::append(const stringx &cp)
{
	int len = cp.my_buf->char_length;
	lock();
	make_room(my_buf->char_length + cp.my_buf->char_length);
	memcpy(chars + my_buf->char_length, cp.my_buf->get_data(), len);
	bool cache_it = false;
	if (my_buf->char_length == 0)
		cache_it = true;
	my_buf->set_char_length(my_buf->char_length + len);
	my_buf->null_terminate();
#ifdef FASCIST_CACHING
	if (!aggressively_cache_buffer())
		// nested
#endif
		if (cache_it)
			add_buf_to_cache(my_buf);
}


int stringx::find(const char *str) const
{
	int sl;
	sl = strlen(str);
	for (int i = 0; i <= my_buf->char_length - sl; i++) {
		int c;
		for (c = 0; c < sl; c++) {
			if (chars[i+c] != str[c]) break;
		}
		if (c == sl) return i;
	}
	
	return -1;
}


int stringx::rfind(const char *str) const
{
	int sl;
	sl = strlen(str);
	for (int i = my_buf->char_length - sl; i >= 0; i--) {
		int c;
		for (c = 0; c < sl; c++) {
			if (chars[i+c] != str[c]) break;
		}
		if (c == sl) return i;
	}
	
	return -1;
}


int stringx::find(int pos, char c) const
{
	int i;
	
	for (i = pos; i < my_buf->char_length; i++) {
		if (chars[i] == c) return i;
	}
	
	return -1;
}

int stringx::rfind(char c, int pos) const
{
	if (pos == my_buf->char_length) pos--;
	if (pos < 0) return -1;
	assert(pos >= 0 && pos <= my_buf->char_length);
	int i;
	
	for (i = pos; i >= 0; i--) {
		if (chars[i] == c) return i;
	}
	
	return -1;
}


int stringx::rfind(char c) const
{
	return rfind(c, my_buf->char_length - 1);
}



void stringx::to_upper()
{
	lock();
	for (int i = 0; i < my_buf->char_length; i++) {
		chars[i] = toupper(chars[i]);
	}
}


void stringx::to_lower()
{
	lock();
	for (int i = 0; i < my_buf->char_length; i++) {
		chars[i] = tolower(chars[i]);
	}
}


stringx stringx::substr(int i, int n) const
{
	if (i == 0 && n == -1)
		return stringx(*this);
	
	if (n == -1 || n > my_buf->char_length - i) {
		n = my_buf->char_length - i;
	}
	
	if (i > my_buf->char_length)
		return stringx("");
	
	stringx foo;
	foo.append(chars + i, n);
	
	return foo;
}

void stringx::debug_dump_strings()
{
	int i;
	
	for (i = 0; i < STRINGX_TOTAL_STRINGS; i++) {
#ifdef ARCH_ENGINE
		debug_print(
			"String %i (ref %i, chars %i): %s\n",
			i,
			strings[i].get_ref(),
			strings[i].char_length, 
			(char *)strings[i].get_data());
#endif
	}
}

void stringx::remove_leading(const char *remove)
{
	int start;
	for (start = 0; start < length() && strchr(remove, chars[start]) != NULL; start++) continue;
	stringx result = slice(start, length());
	copy(result);
}

void stringx::remove_trailing(const char *remove)
{
	int end;
	for (end = length(); end > 0 && strchr(remove, chars[end-1]) != NULL; end--) continue;
	stringx result = slice(0, end);
	copy(result);
}

stringx stringx::read_token(char *delim, int start_index, bool ignore_leading, bool chop)
{
	int start = start_index, end;
	
	if (start_index < 0)
		start_index += length();
	
	if (ignore_leading) {
		while (strchr(delim, chars[start]) && start < length())
			start++;
	}
	
	end = start;
	
	while (!strchr(delim, chars[end]) && end < length())
		end++;
	
	stringx result = slice(start, end);
	
	if (chop) {
		stringx tmp = slice(end, length());
		copy(tmp);
	}
	
	return result;
}
