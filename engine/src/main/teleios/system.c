#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/system.h"
#include "teleios/globals.h"

// #####################################################################################################################
//
//                                                       ECS
//
// #####################################################################################################################
static TLUlidGenerator *generator;

TLUlid* tl_ecs_create(void) {
    BKS_TRACE_PUSH
    if (generator == NULL) {
        generator = tl_ulid_generator_init(global->application.scene.arena, ULID_PARANOID);
    }

    for (u16 i = 0; i < TL_SCENE_MAX_ACTORS; ++i) {
        if (global->application.ecs.entity[i] != NULL) continue;
        global->application.ecs.entity[i] = tl_ulid_generate(global->application.scene.arena, generator);
        BKS_TRACE_POPV(global->application.ecs.entity[i])
    }

    BKSWARN("Failed to generate entity: array is full")
    BKS_TRACE_POPV(NULL)
}

void tl_ecs_destroy(TLUlid *entity) {
    BKS_TRACE_PUSHA("0x%p", entity)
    if (entity == NULL) BKS_TRACE_POP

    for (u16 i = 0; i < TL_SCENE_MAX_ACTORS; ++i) {
        if (global->application.ecs.entity[i] != NULL && tl_char_equals(global->application.ecs.entity[i]->text, entity->text)) {
            global->application.ecs.entity[i] = NULL;
            BKS_TRACE_POP
        }
    }

    BKSWARN("Failed to destroy entity: not found")
    BKS_TRACE_POP
}
/*
#define TL_ECS_ATTACH_FUNCTION(type)                                                \
    static u16 tl_ecs_attach_##type(TLUlid *entity) {                               \
        BKS_TRACE_PUSHA("0x%p, %d", entity)                                          \
        for (u16 i = 0; i < TL_SCENE_MAX_ACTORS; ++i) {                             \
            if (global->application.ecs.components.type[i].entity == NULL) {        \
                global->application.ecs.components.type[i].entity = entity;         \
                BKS_TRACE_POPV(i)                                                    \
            }                                                                       \
        }                                                                           \
                                                                                    \
        BKSWARN("Failed to generate ecs.components.%s: array is full", #type)        \
        BKS_TRACE_POPV(U16_MAX) \
    }

TL_ECS_ATTACH_FUNCTION(yaml)
TL_ECS_ATTACH_FUNCTION(name)

u16 tl_ecs_attach(TLUlid *entity, const TLEcsComponentType type) {
    BKS_TRACE_PUSHA("0x%p, %d", entity, type)
    u16 index = U16_MAX;
    switch (type) {
        case TL_ECS_COMPONENT_YAML: index = tl_ecs_attach_yaml(entity); break;
        case TL_ECS_COMPONENT_NAME: index = tl_ecs_attach_name(entity); break;
    }

    BKS_TRACE_POPV(index);
}
*/
// #####################################################################################################################
//
//                                                     SCENE
//
// #####################################################################################################################
