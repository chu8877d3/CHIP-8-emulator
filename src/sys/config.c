#include "config.h"
#include <stdio.h>

static char* read_json_file()
{
}

AppConfig config_init()
{
    AppConfig config = CONFIG_DEFAULT;
    bool file_exists = true;
    FILE* fp = fopen("config.json", "r");
    if (fp == NULL) {
        file_exists = false;
    } else {
    }

    return config;
}
