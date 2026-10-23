#ifndef __MMCE_SUPPORT_H
#define __MMCE_SUPPORT_H

#include "include/iosupport.h"

// Values of gMMCESlot: the memory-card slot of the MMCE card, or the first port that answers a ping.
#define MMCE_SLOT_1    0
#define MMCE_SLOT_2    1
#define MMCE_SLOT_AUTO 2

void mmceInit(item_list_t *itemList);
item_list_t *mmceGetObject(int initOnly);
void mmceLoadModules(void);
void mmceLaunchGame(item_list_t *itemList, int id, config_set_t *configSet);

#endif
