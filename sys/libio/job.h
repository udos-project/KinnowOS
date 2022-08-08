#ifndef __LIBIO_JOB_H__
#define __LIBIO_JOB_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct job_stats {
    size_t used_size;
    size_t free_size;
    size_t n_regions;
};

void job_get_stats(struct job_stats *stats);

#ifdef __cplusplus
}
#endif

#endif
