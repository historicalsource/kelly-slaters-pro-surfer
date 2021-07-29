#if !defined( _TRIPLE_H_ ) && !defined( TARGET_MKS )
#define _TRIPLE_H_

template <class T1, class T2, class T3>
struct triple
{
  typedef T1 first_type;
  typedef T2 second_type;
  typedef T3 third_type;

  T1 first;
  T2 second;
  T3 third;
  triple() : first( T1() ), second( T2() ), third( T3() ) {}
  triple( const T1& a, const T2& b, const T3& c ) : first( a ), second( b ), third( c ) {}
};

template <class T1, class T2, class T3>
inline bool operator== ( const triple<T1, T2, T3>& x, const triple<T1, T2, T3>& y )
{ 
  return x.first == y.first && x.second == y.second && x.third == y.third; 
}

template <class T1, class T2, class T3>
inline bool operator< ( const triple<T1, T2, T3>& x, const triple<T1, T2, T3>& y )
{ 
  return x.first < y.first || 
         ( !(y.first < x.first) && x.second < y.second ) ||
         ( !(y.first < x.first) && !(y.second < x.second) && x.third < y.third ); 
}

#endif // !defined( _TRIPLE_H_ ) && !defined( TARGET_MKS )
