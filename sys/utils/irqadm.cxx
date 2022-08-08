/* iradm.c
 *
 * IRQ Administrator, useless and unused
 *
 * Implements algorithms for managing IRQ lines of a system - not nescesarily
 * the x86 IRQ lines are the ones controlled by this system, those can also be
 * table-like-IRQ based systems (i.e RISC-V with the vector table)
 * 
 * The IRQ "hardware" assignation and other stuff that directly talks to the
 * hardware is dependent on the arch's irq.c and irq.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef unsigned int irq_t;
typedef void (*irq_handler_t)(size_t id);

typedef enum _irq_type {
    IRQ_TYPE_SHARED = 0,
    IRQ_TYPE_PRIVATE = 1
}irq_type_t;

struct irq_line {
    irq_type_t type;
    irq_handler_t *handlers;
    size_t n_handlers;
};

struct irq_range {
    struct irq_line *lines;
    size_t n_lines;
    size_t start;
    void (*alloc)(irq_t irq);
    void (*free)(irq_t irq);
};

struct irq_range * irq_range_create(size_t start, size_t n_lines, void (*_alloc)(irq_t irq), void (*_free)(irq_t irq));
irq_t  irq_alloc_irq(irq_handler_t *handler);
void irq_free_irq(irq_t id);
void irq_range_delete(struct irq_range *range);

struct irq_range_list {
    struct irq_range *ranges;
    size_t n_ranges;
};

static irq_range_list g_irq_table = {0};

static irq_range *irq_range_add(irq_range *range)
{
    g_irq_table.ranges = (irq_range *)realloc(g_irq_table.ranges, (g_irq_table.n_ranges + 1) * sizeof(irq_range));
    if(g_irq_table.ranges == nullptr) {
        printf("Out of memory");
        return nullptr;
    }
    memcpy(&g_irq_table.ranges[g_irq_table.n_ranges++], range, sizeof(irq_range));
    return &g_irq_table.ranges[g_irq_table.n_ranges - 1];
}

irq_range *irq_range_create(size_t start, size_t n_lines, void (*_alloc)(irq_t irq), void (*_free)(irq_t irq))
{
    auto *range = (irq_range *)malloc(sizeof(irq_range));
    if(range == nullptr) {
        printf("Out of memory");
        return nullptr;
    }

    range->n_lines = n_lines;
    range->start = start;

    range->lines = (irq_line *)calloc(range->n_lines, sizeof(irq_line));
    if(range->lines == nullptr) {
        printf("Out of memory");
        free(range);
        return nullptr;
    }
    range->alloc = _alloc;
    range->free = _free;
    return irq_range_add(range);
}

static irq_handler_t *irq_line_add_handler(irq_line *line, irq_handler_t *handler)
{
    line->handlers = (irq_handler_t *)realloc(line->handlers, (line->n_handlers + 1) * sizeof(irq_line));
    if(line->handlers == nullptr) {
        printf("Out of memory");
        return nullptr;
    }
    memcpy(&line->handlers[line->n_handlers++], handler, sizeof(irq_handler_t));
    return &line->handlers[line->n_handlers - 1];
}

static irq_t irq_range_alloc_irq(irq_range *range, irq_handler_t *handler)
{
    for(size_t i = 0; i < range->n_lines; i++) {
        auto *line = &range->lines[i];
        /* We cannot allocate on private lines */
        if(line->type == IRQ_TYPE_PRIVATE) continue;

        irq_line_add_handler(line, handler);
        return (irq_t)i + (irq_t)range->start;
    }
    return (irq_t)-1;
}

irq_t irq_alloc_irq(irq_handler_t *handler)
{
    for(size_t i = 0; i < g_irq_table.n_ranges; i++) {
        auto *range = &g_irq_table.ranges[i];
        irq_t irq =  irq_range_alloc_irq(range, handler);
        if(irq == (irq_t)-1) continue;

        /* Sucessfully allocated handler - we are going to call the "alloc"
         * function for this range to notify whichever driver controls
         * this range of the new allocation */
        if(range->alloc == nullptr) {
            printf("Warning: IRQ range has no allocator handler");
            return irq;
        }
        range->alloc(irq);
        return irq;
    }
    return (irq_t)-1;
}

void irq_free_irq(irq_t irq)
{
    for(size_t i = 0; i < g_irq_table.n_ranges; i++) {
        auto *range = &g_irq_table.ranges[i];
        /* Check that id is between the range */
        if(irq >= range->start && irq <= range->start + range->n_lines) {
            free(range->lines[irq - range->start].handlers);
            range->lines[irq - range->start].n_handlers = 0;
            range->lines[irq - range->start].type = IRQ_TYPE_SHARED;

            if(range->free == nullptr) {
                printf("Warning: IRQ range has no deallocator handler");
                return;
            }
            range->free(irq);
            return;
        }
    }
}

void irq_range_delete(struct irq_range *range)
{
    free(range->lines);
    free(range);
}

#define VERSION_STRING "v1.0"

int main(int argc, char **argv)
{
    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "/VERSION")) {
            printf("Interrupt Request Query Administrator (IRQADM) " VERSION_STRING "\r\n");
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "Unknown option %s\r\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    /** @todo Run as daemon */
    while(1) {

    }

    exit(EXIT_SUCCESS);
    return 0;
}
