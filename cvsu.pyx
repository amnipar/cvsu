import cython
import numpy as np
cimport numpy as np
cimport cvsu

DTYPE = np.float32
ctypedef np.float32_t DTYPE_t

@cython.boundscheck(False)
@cython.wraparound(False)
def process_image(
    np.ndarray[np.float32_t, ndim=2, mode='c'] src not None,
    np.ndarray[np.float32_t, ndim=3, mode='c'] dst not None):
  assert src.dtype == DTYPE
  assert dst.dtype == DTYPE
  cdef unsigned long rows = src.shape[0]
  cdef unsigned long cols = src.shape[1]
  assert dst.shape[0] == rows
  assert dst.shape[1] == cols
  assert dst.shape[2] == 3
  process_data(<float*>src.data, <float*>dst.data, rows, cols)
  return None

@cython.boundscheck(False)
@cython.wraparound(False)
def run_idiffusion(
    np.ndarray[np.float32_t, ndim=2, mode='c'] src not None,
    np.ndarray[np.float32_t, ndim=2, mode='c'] dst not None,
    unsigned long rounds):
  assert src.dtype == DTYPE
  assert dst.dtype == DTYPE
  cdef unsigned long rows = src.shape[0]
  cdef unsigned long cols = src.shape[1]
  assert dst.shape[0] == rows
  assert dst.shape[1] == cols
  idiffusion(<float*>src.data, <float*>dst.data, rows, cols, rounds)
  return None

@cython.boundscheck(False)
@cython.wraparound(False)
def run_adiffusion(
    np.ndarray[np.float32_t, ndim=2, mode='c'] src not None,
    np.ndarray[np.float32_t, ndim=2, mode='c'] dst not None,
    unsigned long rounds):
  assert src.dtype == DTYPE
  assert dst.dtype == DTYPE
  cdef unsigned long rows = src.shape[0]
  cdef unsigned long cols = src.shape[1]
  assert dst.shape[0] == rows
  assert dst.shape[1] == cols
  adiffusion(<float*>src.data, <float*>dst.data, rows, cols, rounds)
  return None

@cython.boundscheck(False)
@cython.wraparound(False)
def run_msf(
    np.ndarray[np.float32_t, ndim=2, mode='c'] src not None,
    np.ndarray[np.float32_t, ndim=3, mode='c'] dst not None,
    float threshold):
  assert src.dtype == DTYPE
  assert dst.dtype == DTYPE
  cdef unsigned long rows = src.shape[0]
  cdef unsigned long cols = src.shape[1]
  assert dst.shape[0] == rows
  assert dst.shape[1] == cols
  assert dst.shape[2] == 3
  msf(<float*>src.data, <float*>dst.data, rows, cols, threshold)
  return None



@cython.boundscheck(False)
@cython.wraparound(False)
def run_cc(
    np.ndarray[np.uint8_t, ndim=2, mode='c'] src not None,
    np.ndarray[np.uint8_t, ndim=3, mode='c'] dst not None):
  cdef unsigned long rows = src.shape[0]
  cdef unsigned long cols = src.shape[1]
  assert dst.shape[0] == rows
  assert dst.shape[1] == cols
  assert dst.shape[2] == 3
  cc(<unsigned char*>src.data, <unsigned char*>dst.data, rows, cols)
  return None
