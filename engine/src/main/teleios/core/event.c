#include "teleios/core.h"

static TLList* subscribers[U16_MAX] = { 0 };

void tl_event_subscribe(const u16 event, const PFN_handler handler) {
    TLSTACKPUSHA("%u, 0x%p", event, handler)

    if (subscribers[event] == NULL) {
        TLVERBOSE("Creating subscriber list for event %u", event)
        subscribers[event] = tl_list_create(runtime->arenas.permanent);
    }
    
    tl_list_add(subscribers[event], handler);
    TLVERBOSE("Event %u has %u subscribers", event, tl_list_length(subscribers[event]));
    
    TLSTACKPOP
}

static void tl_event_process(void *data) {

}

void tl_event_submmit(u16 event, void* data) {
    TLSTACKPUSHA("%u, 0x%p", event, data)
    if (subscribers[event] == NULL) {
        TLVERBOSE("No listener available for event %u", event)
        TLSTACKPOP
        return;
    }
    
    TLIterator* it = tl_list_iterator_create(subscribers[event]);
    for (PFN_handler handler = tl_list_iterator_next(it) ; handler != NULL ; handler = tl_list_iterator_next(it)) {
        if (handler(data)) {
            break;
        }
    }

    TLSTACKPOP
}