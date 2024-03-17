/* simple_comm_buff.h - Simple Communication Buffer
                        Interface of the simple communication buffer. */

#ifndef _SIMPLE_COMM_BUFF_H
#define _SIMPLE_COMM_BUFF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "cflp_udf.h"

#ifdef __cplusplus
extern "C" {
#endif

class simple_comm_buff{
private:
  CHR *_simple_comm_buff;
  sem_t wr_sem;
  sem_t rd_sem;
public:
  simple_comm_buff();
  ~simple_comm_buff();
  void *operator new(size_t size);
  void *operator new[](size_t size);
  void operator delete(void *p);
  void operator delete[](void *p);
  CHR write(const CHR *str_data);
  CHR *read(CHR **str_data);
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* simple_comm_buff.h */

