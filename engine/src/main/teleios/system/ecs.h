#ifndef __TELEIOS_SYSTEM_ECS__
#define __TELEIOS_SYSTEM_ECS__

#include "teleios/defines.h"

typedef enum {
    TL_ECS_COMPONENT_YAML,
    TL_ECS_COMPONENT_NAME,
} TLEcsComponentType;

TLUlid* tl_ecs_create(void);
void tl_ecs_destroy(TLUlid *entity);

u16 tl_ecs_attach(TLUlid *entity, TLEcsComponentType type);

#endif //__TELEIOS_SYSTEM_ECS__
