#include <job.h>
#include <svc.h>
#include <assert.h>

constinit program_data_block pdb_area = {};

STDAPI void job_get_stats(struct job_stats *stats)
{
    io_svc(SVC_GET_PDB, (uintptr_t)&pdb_area, 0, 0);
    stats->free_size = pdb_area.pdb_free_bytes;
    stats->used_size = pdb_area.pdb_used_bytes;
    stats->n_regions = 1;
}
