#include "teleios/core/event.h"
#include "teleios/core/memory.h"
#include "teleios/core/logger.h"
#include "teleios/core/container.h"

static TLList* subscribers[U16_MAX] = { 0 };

void tl_event_subscribe(u16 event, PFN_handler handler) {
    TLTRACE(">> tl_event_subscribe(%u, ...)", event)

    if (subscribers[event] == NULL) {
        TLVERBOSE("Creating subscriber list for event %u", event)
        subscribers[event] = tl_list_create(global_arena);
    }
    
    tl_list_add(subscribers[event], handler);
    TLVERBOSE("Event %u has %u subscribers", event, tl_list_length(subscribers[event]));
    
    TLTRACE("<< tl_event_subscribe(%u, ...)", event)
}

static void tl_event_process(void *data) {

}

void tl_event_submmit(u16 event, void* data) {
    TLTRACE(">> tl_event_submmit(%u, ...)", event)
    if (subscribers[event] == NULL) {
        TLVERBOSE("No listener available for event %u", event)
        return;
    }
    
    TLIterator* it = tl_list_iterator_create(subscribers[event]);
    for (PFN_handler handler = tl_list_iterator_next(it) ; handler != NULL ; handler = tl_list_iterator_next(it)) {
        if (handler(data)) {
            break;
        }
    }

    TLTRACE("<< tl_event_submmit(%u, ...)", event)
}