#include "teleios/teleios.h"

static TLAllocator* m_allocator;
static TLMap* m_properties;

static void tl_serializer_walk();

b8 tl_config_initialize() {
    TL_PROFILER_PUSH
    m_allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    m_properties = tl_map_create(m_allocator, 32, false);
    tl_serializer_walk();

    tl_logger_set_level(tl_config_get_log_level("teleios.logging.level"));
    TL_PROFILER_POP_WITH(true)
}

#include "teleios/config/getters.inl"
#include "teleios/config/parser.inl"
#include "teleios/scene/loader.inl"

b8 tl_config_terminate(void) {
    TL_PROFILER_PUSH

    if (m_properties != NULL) {
        tl_map_destroy(m_properties);
        m_properties = NULL;
    }

    if (m_allocator != NULL) {
        tl_memory_allocator_destroy(m_allocator);
        m_allocator = NULL;
    }

    TL_PROFILER_POP_WITH(true)
}

