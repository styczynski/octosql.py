#if PY_MAJOR_VERSION >= 3
#define PY3K
#endif

#define dbg if (DEBUG)
#define dbgm if (DEBUG) std::cout.flush(); \
    if (DEBUG) std::cout << "\n" <<