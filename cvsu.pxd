cdef extern from "graph.h":
  void process_data(float *src, float *dst, unsigned long rows, unsigned long cols)
  void idiffusion(float *src, float *dst, unsigned long rows, unsigned long cols, unsigned long rounds)
  void adiffusion(float *src, float *dst, unsigned long rows, unsigned long cols, unsigned long rounds)
  void msf(float *src, float *dst, unsigned long rows, unsigned long cols, float threshold)
  void cc(unsigned char *src, unsigned char *dst, unsigned long rows, unsigned long cols)
