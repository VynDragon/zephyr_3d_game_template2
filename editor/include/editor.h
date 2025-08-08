#pragma once

extern const struct device *display_devices[DT_ZEPHYR_DISPLAYS_COUNT];

int init_editor_render(void);
int init_editor_UI(void);
int do_editor_UI(void);
