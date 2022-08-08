#ifndef __LIBIO_CSS_H__
#define __LIBIO_CSS_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <svc.h>

/* Empty structs for type-checking */
struct css_device { char fill; };
struct css_request { char fill; };

/* Command chain word flags */
#define CSS_CCW_CD ((1) << S390_BIT(8, 0))
#define CSS_CCW_CC ((1) << S390_BIT(8, 1))
#define CSS_CCW_SLI ((1) << S390_BIT(8, 2))
#define CSS_CCW_SPK ((1) << S390_BIT(8, 3))
#define CSS_CCW_PCI ((1) << S390_BIT(8, 4))
#define CSS_CCW_IDA ((1) << S390_BIT(8, 5))
#define CSS_CCW_S ((1) << S390_BIT(8, 6))
#define CSS_CCW_MIDA ((1) << S390_BIT(8, 7))

enum css_request_flags {
    CSS_REQUEST_MODIFY = 0x01,
    CSS_REQUEST_IGNORE_CC = 0x02,
    CSS_REQUEST_WAIT_ATTENTION = 0x04,
    /* Set by the spooler to indicate the request is done */
    CSS_REQUEST_DONE = 0x10,
};

enum css_status {
    CSS_STATUS_OK = 0,
    CSS_STATUS_PENDING = 1,
    CSS_STATUS_NOT_PRESENT = 3,
};

enum css_cmd {
    /* General purpouse channel subsystem command codes
    * See z/Architecture Principles of Operation, Page 29, Figure 15-5 */
    CSS_CMD_WRITE = 0x01,
    CSS_CMD_READ = 0x02,
    CSS_CMD_READ_BACKWARDS = 0x0C,
    CSS_CMD_CONTROL = 0x03,
    /* Obtain basic sense information from device (for identifying the type of
    * device of course) */
    CSS_CMD_SENSE = 0x04,
    CSS_CMD_SENSE_ID = 0xE4,
    /* Transfer in Chnannel - Usuaully to retry last failed operation */
    CSS_CMD_TIC = 0x08
    /* "Enable this device" */
};

struct css_device *css_get_device(uint16_t id, uint16_t num);
int css_device_query(struct css_device *dev, int query_type);
struct css_request *css_request_create(struct css_device *dev, size_t n_ccws);
int css_request_set_flags(struct css_request *req, int flags);
int css_request_set_ccw(struct css_request *req, struct css_req_set_ccw_data data);
void css_request_await(struct css_request *req);
int css_request_status(struct css_request *req);
void css_request_send(struct css_request *req);
void css_request_delete(struct css_request *req);

#ifdef __cplusplus
}
#endif

#endif
