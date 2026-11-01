#ifndef __XMB_H
#define __XMB_H

#include "include/menusys.h"
#include "include/themes.h"

void xmbDrawBackground(theme_element_t *elem);
void xmbDrawBar(struct menu_list *menu, theme_element_t *elem);
void xmbDrawList(struct menu_list *menu, struct submenu_list *item, theme_element_t *elem);

#endif
