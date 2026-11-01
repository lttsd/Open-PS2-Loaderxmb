#include "include/opl.h"
#include "include/xmb.h"
#include "include/renderman.h"
#include "include/fntsys.h"
#include "include/menusys.h"
#include "include/themes.h"
#include <math.h>

#define XMB_PITCH 34
#define XMB_ICON  48

static int tick;
static float barPos, listOff, listFade = 1;
static struct menu_list *lastMenu;
static struct submenu_list *lastItem;

static int clampi(int v, int lo, int hi)
{
    return v < lo ? lo : v > hi ? hi : v;
}

static u64 col(int r, int g, int b, int a)
{
    return GS_SETREG_RGBA(clampi(r, 0, 255), clampi(g, 0, 255), clampi(b, 0, 255), clampi(a, 0, 0x80));
}

void xmbDrawBackground(theme_element_t *elem)
{
    static const struct {
        float base, amp, speed;
        int alpha;
    } wave[3] = {{300, 40, 1.0f, 0x14}, {350, 32, 1.35f, 0x12}, {405, 26, 1.7f, 0x16}};
    int sw, sh;
    int r = elem->color & 0xff, g = (elem->color >> 8) & 0xff, b = (elem->color >> 16) & 0xff;
    float t = tick++ * 0.012f;

    rmGetScreenExtents(&sw, &sh);

    for (int y = 0; y < sh; y += 8) {
        float k = 0.2f + 0.8f * y / sh;

        rmDrawRect(0, y, sw, 8, col(r * k, g * k, b * k, 0x80));
    }

    for (int l = 0; l < 3; l++) {
        float ph = t * wave[l].speed + l * 2.1f;

        for (int x = 0; x < sw; x += 6) {
            float y = wave[l].base * sh / 480 + sinf(x * 0.011f + ph) * wave[l].amp +
                      sinf(x * 0.027f - ph * 1.4f) * wave[l].amp * 0.35f;

            rmDrawRect(x, y, 6, sh - y, col(r + 90, g + 110, b + 130, wave[l].alpha));
            rmDrawRect(x, y, 6, 2, col(r + 150, g + 170, b + 190, wave[l].alpha * 2));
        }
    }
}

void xmbDrawBar(struct menu_list *menu, theme_element_t *elem)
{
    struct menu_list *first = menu, *m;
    int sw, sh, n = 0, sel = 0, icon = elem->height > 0 ? elem->height : XMB_ICON;

    rmGetScreenExtents(&sw, &sh);
    while (first->prev)
        first = first->prev;

    for (m = first; m; m = m->next) {
        if (!m->item->visible)
            continue;
        if (m == menu)
            sel = n;
        n++;
    }

    if (menu != lastMenu) {
        lastMenu = menu;
        listFade = 0;
    }
    barPos += (sel - barPos) * 0.22f;
    if (fabsf(sel - barPos) < 0.01f)
        barPos = sel;

    n = 0;
    for (m = first; m; m = m->next) {
        float d, focus, x;
        int size, a;
        GSTEXTURE *tex;

        if (!m->item->visible)
            continue;
        d = n++ - barPos;
        x = elem->posX + d * elem->width;
        if (x < -icon || x > sw + icon)
            continue;

        focus = 1 - fminf(1, fabsf(d));
        size = icon * (1 + 0.3f * focus);
        a = 0x30 + (int)(0x50 * focus);

        tex = thmGetTexture(m->item->icon_id);
        if (tex && tex->Mem)
            rmDrawPixmap(tex, x, elem->posY, ALIGN_CENTER, size, size, SCALING_RATIO, GS_SETREG_RGBA(0x80, 0x80, 0x80, a));

        if (focus > 0.4f)
            fntRenderString(elem->font, x, elem->posY + size / 2 + 18, ALIGN_CENTER, 0, 0, menuItemGetText(m->item),
                            col(255, 255, 255, (int)((focus - 0.4f) / 0.6f * 0x80)));
    }
}

void xmbDrawList(struct menu_list *menu, struct submenu_list *item, theme_element_t *elem)
{
    if (!item)
        return;

    if (item != lastItem) {
        if (lastItem && item == lastItem->next)
            listOff += 1;
        else if (lastItem && item == lastItem->prev)
            listOff -= 1;
        lastItem = item;
    }
    listOff *= 0.62f;
    if (fabsf(listOff) < 0.02f)
        listOff = 0;
    listFade = fminf(1, listFade + 0.12f);

    for (int r = -1; r <= 6; r++) {
        struct submenu_list *it = item;
        float pos = r + listOff, d = fabsf(pos), focus = 1 - fminf(1, d);
        int k = r, a;

        while (k < 0 && it) {
            it = it->prev;
            k++;
        }
        while (k > 0 && it) {
            it = it->next;
            k--;
        }
        if (!it)
            continue;

        a = (int)((0x3c + 0x44 * focus) * listFade);
        if (d > 3)
            a = (int)(a * fmaxf(0, 1 - (d - 3) / 2));
        if (a <= 0)
            continue;

        {
            int y = elem->posY + (int)(pos * XMB_PITCH) + (pos > 0.5f ? 10 : 0);
            const char *text = submenuItemGetText(&it->item);

            fntRenderString(elem->font, elem->posX, y, ALIGN_NONE, elem->width, elem->height, text, col(255, 255, 255, a));
            if (focus > 0.7f)
                fntRenderString(elem->font, elem->posX + 1, y, ALIGN_NONE, elem->width, elem->height, text, col(255, 255, 255, a / 3));
        }
    }
}
