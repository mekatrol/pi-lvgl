#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lvgl.h"

static lv_display_t * init_display(void)
{
    const char * card = getenv("LV_LINUX_DRM_CARD");
    if(card == NULL) {
        card = lv_linux_drm_find_device_path();
    }

    lv_display_t * disp = lv_linux_drm_create();
    if(disp == NULL) {
        return NULL;
    }

    lv_linux_drm_set_file(disp, card, -1);
    return disp;
}

static int init_touch(lv_display_t * disp)
{
    const char * pointer = getenv("LV_LINUX_EVDEV_POINTER_DEVICE");
    if(pointer == NULL) {
        fprintf(stderr, "Set LV_LINUX_EVDEV_POINTER_DEVICE to your touchscreen event device\n");
        return -1;
    }

    lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, pointer);
    if(indev == NULL) {
        return -1;
    }

    lv_indev_set_display(indev, disp);
    return 0;
}

static void app_create(void)
{
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "TOUCH DISPLAY");
    lv_obj_center(label);
}

int main(void)
{
    lv_init();

    lv_display_t * disp = init_display();
    if(disp == NULL) {
        fprintf(stderr, "Failed to initialize DRM display\n");
        return EXIT_FAILURE;
    }

    if(init_touch(disp) != 0) {
        fprintf(stderr, "Failed to initialize EVDEV touch\n");
        return EXIT_FAILURE;
    }

    app_create();

    while(1) {
        uint32_t idle_ms = lv_timer_handler();
        usleep((useconds_t)idle_ms * 1000U);
    }
}
