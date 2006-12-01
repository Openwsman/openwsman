
#define NO_HMAP
#define NO_CONFIG
#define NO_LOG
#ifdef _WIN32
#define OS_WIN

#else
#define OS_UNIX
#endif


#  ifdef __GNUC__
#    define INLINE __inline__
#  elif _WIN32
#    define INLINE __inline
#  endif

