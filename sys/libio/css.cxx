#include <stdio.h>
#include <css.h>
#include <assert.h>
#include <svc.h>

STDAPI struct css_device *css_get_device(uint16_t id, uint16_t num)
{
    struct css_device *dev;
    assert(id != 0);
    dev = (struct css_device *)io_svc(SVC_CSS_GET_DEV, (uintptr_t)((id << 16) | num), 0, 0);
    return dev;
}

STDAPI int css_device_query(struct css_device *dev, int query_type)
{
    int r;
    assert(dev != NULL);
    r = (int)io_svc(SVC_CSS_DEVICE_QUERY, (uintptr_t)dev, query_type, 0);
    return r;
}

STDAPI struct css_request *css_request_create(struct css_device *dev, size_t n_ccws)
{
    struct css_request *req;
    assert(dev != NULL);
    req = (struct css_request *)io_svc(SVC_CSS_REQ_CREAT, (uintptr_t)dev, (uintptr_t)n_ccws, 0);
    return req;
}

STDAPI int css_request_set_flags(struct css_request *req, int flags)
{
    assert(req != NULL);
    io_svc(SVC_CSS_REQ_XCHG_FLAGS, (uintptr_t)req, (uintptr_t)flags, 0);
    return 0;
}

STDAPI int css_request_set_ccw(struct css_request *req, struct css_req_set_ccw_data data)
{
    assert(req != NULL);
    io_svc(SVC_CSS_REQ_SET_CCW, (uintptr_t)req, (uintptr_t)&data, 0);
    return 0;
}

STDAPI void css_request_await(struct css_request *req)
{
    int r;
    assert(req != NULL);
    r = (int)io_svc(SVC_CSS_REQ_AWAIT, (uintptr_t)req, 0, 0);
    while(r == 0) {
        r = (int)io_svc(SVC_CSS_REQ_AWAIT, (uintptr_t)req, 0, 0);
    }
    return;
}

STDAPI int css_request_status(struct css_request *req)
{
    int r;
    assert(req != NULL);
    r = (int)io_svc(SVC_CSS_REQ_GET_STATUS, (uintptr_t)req, 0, 0);
    return r;
}

STDAPI void css_request_send(struct css_request *req)
{
    assert(req != NULL);
    dprintf("Sending request %p\r\n", req);
    io_svc(SVC_CSS_REQ_SEND, (uintptr_t)req, 0, 0);
    return;
}

STDAPI void css_request_delete(struct css_request *req)
{
    assert(req != NULL);
    io_svc(SVC_CSS_REQ_DELETE, (uintptr_t)req, 0, 0);
    return;
}
