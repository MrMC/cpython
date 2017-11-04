#include "Python.h"
#ifdef MS_WINDOWS
#  include <windows.h>
/* All sample MSDN wincrypt programs include the header below. It is at least
 * required with Min GW. */
#  include <wincrypt.h>
#else
#  include <fcntl.h>
#  ifdef HAVE_SYS_STAT_H
#    include <sys/stat.h>
#  endif
#  ifdef HAVE_LINUX_RANDOM_H
#    include <linux/random.h>
#  endif
#  if defined(HAVE_SYS_RANDOM_H) && (defined(HAVE_GETRANDOM) || defined(HAVE_GETENTROPY))
#    include <sys/random.h>
#  endif
#  if !defined(HAVE_GETRANDOM) && defined(HAVE_GETRANDOM_SYSCALL)
#    include <sys/syscall.h>
#  endif
#endif

#ifdef __APPLE__
#  include "TargetConditionals.h"
#endif /* __APPLE__ */

#ifdef Py_DEBUG
int _Py_HashSecret_Initialized = 0;
#else
static int _Py_HashSecret_Initialized = 0;
#endif

#ifdef MS_WINDOWS
static HCRYPTPROV hCryptProv = 0;

static int
win32_urandom_init(int raise)
{
    /* Acquire context */
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL,
                             PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        goto error;

    return 0;

error:
    if (raise) {
        PyErr_SetFromWindowsErr(0);
    }
    return -1;
}

/* Fill buffer with size pseudo-random bytes generated by the Windows CryptoGen
   API. Return 0 on success, or raise an exception and return -1 on error. */
static int
win32_urandom(unsigned char *buffer, Py_ssize_t size, int raise)
{
    Py_ssize_t chunk;

    if (hCryptProv == 0)
    {
        if (win32_urandom_init(raise) == -1) {
            return -1;
        }
    }

    while (size > 0)
    {
        chunk = size > INT_MAX ? INT_MAX : size;
        if (!CryptGenRandom(hCryptProv, (DWORD)chunk, buffer))
        {
            /* CryptGenRandom() failed */
            if (raise) {
                PyErr_SetFromWindowsErr(0);
            }
            return -1;
        }
        buffer += chunk;
        size -= chunk;
    }
    return 0;
}

#else /* !MS_WINDOWS */

#if defined(HAVE_GETRANDOM) || defined(HAVE_GETRANDOM_SYSCALL)
#define PY_GETRANDOM 1

/* Call getrandom() to get random bytes:

   - Return 1 on success
   - Return 0 if getrandom() is not available (failed with ENOSYS or EPERM),
     or if getrandom(GRND_NONBLOCK) failed with EAGAIN (system urandom not
     initialized yet) and raise=0.
   - Raise an exception (if raise is non-zero) and return -1 on error:
     if getrandom() failed with EINTR, raise is non-zero and the Python signal
     handler raised an exception, or if getrandom() failed with a different
     error.

   getrandom() is retried if it failed with EINTR: interrupted by a signal. */
static int
py_getrandom(void *buffer, Py_ssize_t size, int blocking, int raise)
{
    /* Is getrandom() supported by the running kernel? Set to 0 if getrandom()
       failed with ENOSYS or EPERM. Need Linux kernel 3.17 or newer, or Solaris
       11.3 or newer */
    static int getrandom_works = 1;
    int flags;
    char *dest;
    long n;

    if (!getrandom_works) {
        return 0;
    }

    flags = blocking ? 0 : GRND_NONBLOCK;
    dest = buffer;
    while (0 < size) {
#ifdef sun
        /* Issue #26735: On Solaris, getrandom() is limited to returning up
           to 1024 bytes. Call it multiple times if more bytes are
           requested. */
        n = Py_MIN(size, 1024);
#else
        n = Py_MIN(size, LONG_MAX);
#endif

        errno = 0;
#ifdef HAVE_GETRANDOM
        if (raise) {
            Py_BEGIN_ALLOW_THREADS
            n = getrandom(dest, n, flags);
            Py_END_ALLOW_THREADS
        }
        else {
            n = getrandom(dest, n, flags);
        }
#else
        /* On Linux, use the syscall() function because the GNU libc doesn't
           expose the Linux getrandom() syscall yet. See:
           https://sourceware.org/bugzilla/show_bug.cgi?id=17252 */
        if (raise) {
            Py_BEGIN_ALLOW_THREADS
            n = syscall(SYS_getrandom, dest, n, flags);
            Py_END_ALLOW_THREADS
        }
        else {
            n = syscall(SYS_getrandom, dest, n, flags);
        }
#endif

        if (n < 0) {
            /* ENOSYS: the syscall is not supported by the kernel.
               EPERM: the syscall is blocked by a security policy (ex: SECCOMP)
               or something else. */
            if (errno == ENOSYS || errno == EPERM) {
                getrandom_works = 0;
                return 0;
            }

            /* getrandom(GRND_NONBLOCK) fails with EAGAIN if the system urandom
               is not initialiazed yet. For _PyRandom_Init(), we ignore the
               error and fall back on reading /dev/urandom which never blocks,
               even if the system urandom is not initialized yet:
               see the PEP 524. */
            if (errno == EAGAIN && !raise && !blocking) {
                return 0;
            }

            if (errno == EINTR) {
                if (raise) {
                    if (PyErr_CheckSignals()) {
                        return -1;
                    }
                }

                /* retry getrandom() if it was interrupted by a signal */
                continue;
            }

            if (raise) {
                PyErr_SetFromErrno(PyExc_OSError);
            }
            return -1;
        }

        dest += n;
        size -= n;
    }
    return 1;
}

// iOS, tvOS and watchOS have an incomplete definitions of getentropy
// so it is *found* by configure, but doesn't actually exist.
#elif defined(HAVE_GETENTROPY) && !TARGET_OS_IPHONE
#define PY_GETENTROPY 1

/* Fill buffer with size pseudo-random bytes generated by getentropy():

   - Return 1 on success
   - Return 0 if getentropy() syscall is not available (failed with ENOSYS or
     EPERM).
   - Raise an exception (if raise is non-zero) and return -1 on error:
     if getentropy() failed with EINTR, raise is non-zero and the Python signal
     handler raised an exception, or if getentropy() failed with a different
     error.

   getentropy() is retried if it failed with EINTR: interrupted by a signal. */
static int
py_getentropy(char *buffer, Py_ssize_t size, int raise)
{
    /* Is getentropy() supported by the running kernel? Set to 0 if
       getentropy() failed with ENOSYS or EPERM. */
    static int getentropy_works = 1;

    if (!getentropy_works) {
        return 0;
    }

    while (size > 0) {
        /* getentropy() is limited to returning up to 256 bytes. Call it
           multiple times if more bytes are requested. */
        Py_ssize_t len = Py_MIN(size, 256);
        int res;

        if (raise) {
            Py_BEGIN_ALLOW_THREADS
            res = getentropy(buffer, len);
            Py_END_ALLOW_THREADS
        }
        else {
            res = getentropy(buffer, len);
        }

        if (res < 0) {
            /* ENOSYS: the syscall is not supported by the running kernel.
               EPERM: the syscall is blocked by a security policy (ex: SECCOMP)
               or something else. */
            if (errno == ENOSYS || errno == EPERM) {
                getentropy_works = 0;
                return 0;
            }

            if (errno == EINTR) {
                if (raise) {
                    if (PyErr_CheckSignals()) {
                        return -1;
                    }
                }

                /* retry getentropy() if it was interrupted by a signal */
                continue;
            }

            if (raise) {
                PyErr_SetFromErrno(PyExc_OSError);
            }
            return -1;
        }

        buffer += len;
        size -= len;
    }
    return 1;
}
#endif /* defined(HAVE_GETENTROPY) && !defined(sun) */


static struct {
    int fd;
    dev_t st_dev;
    ino_t st_ino;
} urandom_cache = { -1 };

/* Read random bytes from the /dev/urandom device:

   - Return 0 on success
   - Raise an exception (if raise is non-zero) and return -1 on error

   Possible causes of errors:

   - open() failed with ENOENT, ENXIO, ENODEV, EACCES: the /dev/urandom device
     was not found. For example, it was removed manually or not exposed in a
     chroot or container.
   - open() failed with a different error
   - fstat() failed
   - read() failed or returned 0

   read() is retried if it failed with EINTR: interrupted by a signal.

   The file descriptor of the device is kept open between calls to avoid using
   many file descriptors when run in parallel from multiple threads:
   see the issue #18756.

   st_dev and st_ino fields of the file descriptor (from fstat()) are cached to
   check if the file descriptor was replaced by a different file (which is
   likely a bug in the application): see the issue #21207.

   If the file descriptor was closed or replaced, open a new file descriptor
   but don't close the old file descriptor: it probably points to something
   important for some third-party code. */
static int
dev_urandom(char *buffer, Py_ssize_t size, int raise)
{
    int fd;
    Py_ssize_t n;

    if (raise) {
        struct _Py_stat_struct st;

        if (urandom_cache.fd >= 0) {
            /* Does the fd point to the same thing as before? (issue #21207) */
            if (_Py_fstat_noraise(urandom_cache.fd, &st)
                || st.st_dev != urandom_cache.st_dev
                || st.st_ino != urandom_cache.st_ino) {
                /* Something changed: forget the cached fd (but don't close it,
                   since it probably points to something important for some
                   third-party code). */
                urandom_cache.fd = -1;
            }
        }
        if (urandom_cache.fd >= 0)
            fd = urandom_cache.fd;
        else {
            fd = _Py_open("/dev/urandom", O_RDONLY);
            if (fd < 0) {
                if (errno == ENOENT || errno == ENXIO ||
                    errno == ENODEV || errno == EACCES) {
                    PyErr_SetString(PyExc_NotImplementedError,
                                    "/dev/urandom (or equivalent) not found");
                }
                /* otherwise, keep the OSError exception raised by _Py_open() */
                return -1;
            }
            if (urandom_cache.fd >= 0) {
                /* urandom_fd was initialized by another thread while we were
                   not holding the GIL, keep it. */
                close(fd);
                fd = urandom_cache.fd;
            }
            else {
                if (_Py_fstat(fd, &st)) {
                    close(fd);
                    return -1;
                }
                else {
                    urandom_cache.fd = fd;
                    urandom_cache.st_dev = st.st_dev;
                    urandom_cache.st_ino = st.st_ino;
                }
            }
        }

        do {
            n = _Py_read(fd, buffer, (size_t)size);
            if (n == -1)
                return -1;
            if (n == 0) {
                PyErr_Format(PyExc_RuntimeError,
                        "Failed to read %zi bytes from /dev/urandom",
                        size);
                return -1;
            }

            buffer += n;
            size -= n;
        } while (0 < size);
    }
    else {
        fd = _Py_open_noraise("/dev/urandom", O_RDONLY);
        if (fd < 0) {
            return -1;
        }

        while (0 < size)
        {
            do {
                n = read(fd, buffer, (size_t)size);
            } while (n < 0 && errno == EINTR);

            if (n <= 0) {
                /* stop on error or if read(size) returned 0 */
                close(fd);
                return -1;
            }

            buffer += n;
            size -= n;
        }
        close(fd);
    }
    return 0;
}

static void
dev_urandom_close(void)
{
    if (urandom_cache.fd >= 0) {
        close(urandom_cache.fd);
        urandom_cache.fd = -1;
    }
}
#endif /* !MS_WINDOWS */


/* Fill buffer with pseudo-random bytes generated by a linear congruent
   generator (LCG):

       x(n+1) = (x(n) * 214013 + 2531011) % 2^32

   Use bits 23..16 of x(n) to generate a byte. */
static void
lcg_urandom(unsigned int x0, unsigned char *buffer, size_t size)
{
    size_t index;
    unsigned int x;

    x = x0;
    for (index=0; index < size; index++) {
        x *= 214013;
        x += 2531011;
        /* modulo 2 ^ (8 * sizeof(int)) */
        buffer[index] = (x >> 16) & 0xff;
    }
}

/* Read random bytes:

   - Return 0 on success
   - Raise an exception (if raise is non-zero) and return -1 on error

   Used sources of entropy ordered by preference, preferred source first:

   - CryptGenRandom() on Windows
   - getrandom() function (ex: Linux and Solaris): call py_getrandom()
   - getentropy() function (ex: OpenBSD): call py_getentropy()
   - /dev/urandom device

   Read from the /dev/urandom device if getrandom() or getentropy() function
   is not available or does not work.

   Prefer getrandom() over getentropy() because getrandom() supports blocking
   and non-blocking mode: see the PEP 524. Python requires non-blocking RNG at
   startup to initialize its hash secret, but os.urandom() must block until the
   system urandom is initialized (at least on Linux 3.17 and newer).

   Prefer getrandom() and getentropy() over reading directly /dev/urandom
   because these functions don't need file descriptors and so avoid ENFILE or
   EMFILE errors (too many open files): see the issue #18756.

   Only the getrandom() function supports non-blocking mode.

   Only use RNG running in the kernel. They are more secure because it is
   harder to get the internal state of a RNG running in the kernel land than a
   RNG running in the user land. The kernel has a direct access to the hardware
   and has access to hardware RNG, they are used as entropy sources.

   Note: the OpenSSL RAND_pseudo_bytes() function does not automatically reseed
   its RNG on fork(), two child processes (with the same pid) generate the same
   random numbers: see issue #18747. Kernel RNGs don't have this issue,
   they have access to good quality entropy sources.

   If raise is zero:

   - Don't raise an exception on error
   - Don't call the Python signal handler (don't call PyErr_CheckSignals()) if
     a function fails with EINTR: retry directly the interrupted function
   - Don't release the GIL to call functions.
*/
static int
pyurandom(void *buffer, Py_ssize_t size, int blocking, int raise)
{
#if defined(PY_GETRANDOM) || defined(PY_GETENTROPY)
    int res;
#endif

    if (size < 0) {
        if (raise) {
            PyErr_Format(PyExc_ValueError,
                         "negative argument not allowed");
        }
        return -1;
    }

    if (size == 0) {
        return 0;
    }

#ifdef MS_WINDOWS
    return win32_urandom((unsigned char *)buffer, size, raise);
#else

#if defined(PY_GETRANDOM) || defined(PY_GETENTROPY)
#ifdef PY_GETRANDOM
    res = py_getrandom(buffer, size, blocking, raise);
#else
    res = py_getentropy(buffer, size, raise);
#endif
    if (res < 0) {
        return -1;
    }
    if (res == 1) {
        return 0;
    }
    /* getrandom() or getentropy() function is not available: failed with
       ENOSYS or EPERM. Fall back on reading from /dev/urandom. */
#endif

    return dev_urandom(buffer, size, raise);
#endif
}

/* Fill buffer with size pseudo-random bytes from the operating system random
   number generator (RNG). It is suitable for most cryptographic purposes
   except long living private keys for asymmetric encryption.

   On Linux 3.17 and newer, the getrandom() syscall is used in blocking mode:
   block until the system urandom entropy pool is initialized (128 bits are
   collected by the kernel).

   Return 0 on success. Raise an exception and return -1 on error. */
int
_PyOS_URandom(void *buffer, Py_ssize_t size)
{
    return pyurandom(buffer, size, 1, 1);
}

/* Fill buffer with size pseudo-random bytes from the operating system random
   number generator (RNG). It is not suitable for cryptographic purpose.

   On Linux 3.17 and newer (when getrandom() syscall is used), if the system
   urandom is not initialized yet, the function returns "weak" entropy read
   from /dev/urandom.

   Return 0 on success. Raise an exception and return -1 on error. */
int
_PyOS_URandomNonblock(void *buffer, Py_ssize_t size)
{
    return pyurandom(buffer, size, 0, 1);
}

void
_PyRandom_Init(void)
{
    char *env;
    unsigned char *secret = (unsigned char *)&_Py_HashSecret.uc;
    Py_ssize_t secret_size = sizeof(_Py_HashSecret_t);
    Py_BUILD_ASSERT(sizeof(_Py_HashSecret_t) == sizeof(_Py_HashSecret.uc));

    if (_Py_HashSecret_Initialized)
        return;
    _Py_HashSecret_Initialized = 1;

    /*
      Hash randomization is enabled.  Generate a per-process secret,
      using PYTHONHASHSEED if provided.
    */

    env = Py_GETENV("PYTHONHASHSEED");
    if (env && *env != '\0' && strcmp(env, "random") != 0) {
        char *endptr = env;
        unsigned long seed;
        seed = strtoul(env, &endptr, 10);
        if (*endptr != '\0'
            || seed > 4294967295UL
            || (errno == ERANGE && seed == ULONG_MAX))
        {
            Py_FatalError("PYTHONHASHSEED must be \"random\" or an integer "
                          "in range [0; 4294967295]");
        }
        if (seed == 0) {
            /* disable the randomized hash */
            memset(secret, 0, secret_size);
        }
        else {
            lcg_urandom(seed, secret, secret_size);
        }
    }
    else {
        int res;

        /* _PyRandom_Init() is called very early in the Python initialization
           and so exceptions cannot be used (use raise=0).

           _PyRandom_Init() must not block Python initialization: call
           pyurandom() is non-blocking mode (blocking=0): see the PEP 524. */
        res = pyurandom(secret, secret_size, 0, 0);
        if (res < 0) {
            Py_FatalError("failed to get random numbers to initialize Python");
        }
    }
}

void
_PyRandom_Fini(void)
{
#ifdef MS_WINDOWS
    if (hCryptProv) {
        CryptReleaseContext(hCryptProv, 0);
        hCryptProv = 0;
    }
#else
    dev_urandom_close();
#endif
}
