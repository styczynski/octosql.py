#ifndef OCTSQLN_PY_DEBUG
#define OCTSQLN_PY_DEBUG

#if PY_MAJOR_VERSION >= 3
#define PY3K
#endif

#define dbg if (DEBUG)
#define dbgm if (DEBUG) std::cout.flush(); \
    if (DEBUG) std::cout << "\n" <<

#endif // OCTSQLN_PY_DEBUG