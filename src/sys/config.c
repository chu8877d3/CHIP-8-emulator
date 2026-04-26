#include "config.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static cJSON* read_json_file(FILE* fp);
static void write_json_file(FILE* fp, cJSON* root);
static void json_to_struct(cJSON* root, AppConfig* config);
static cJSON* struct_to_json(AppConfig* config);
static void parse_json_color(cJSON* obj, ColorRGBA* out_color);
static void parse_config_color(ColorRGBA* in_color, char* hex_str);

AppConfig config_init()
{
    AppConfig config = CONFIG_DEFAULT;
    FILE* fp = fopen("config.json", "rb");
    if (fp != NULL) {
        cJSON* root = read_json_file(fp);
        if (root != NULL) {
            json_to_struct(root, &config);
        }
        fclose(fp);
    }
    return config;
}
void config_save(AppConfig* config)
{
    (void)config;
    FILE* fp = fopen("config.json", "wb");
    if (fp == NULL) {
        perror("error");
    } else {
        cJSON* root = struct_to_json(config);
        if (root != NULL) {
            write_json_file(fp, root);
        }
        fclose(fp);
    }
}

static cJSON* read_json_file(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    if (size > 0) {
        char* config_data = (char*)malloc(size + 1);
        if (config_data != NULL) {
            size_t read_size = fread(config_data, 1, (size_t)size, fp);
            config_data[read_size] = '\0';

            cJSON* root = cJSON_Parse(config_data);
            free(config_data);
            return root;
        }
    }
    return NULL;
}

static void write_json_file(FILE* fp, cJSON* root)
{
    char* json_str = cJSON_Print(root);
    size_t byte_length = strlen(json_str);
    if (byte_length > 0) {
        fwrite(json_str, 1, byte_length, fp);
    }
    cJSON_free(json_str);
    cJSON_Delete(root);
}

static void json_to_struct(cJSON* root, AppConfig* config)
{
    cJSON* emulator = cJSON_GetObjectItem(root, "emulator");
    cJSON* display = cJSON_GetObjectItem(root, "display");
    cJSON* input = cJSON_GetObjectItem(root, "input");

    if (emulator) {
        cJSON* cpu_frequency_hz = cJSON_GetObjectItem(emulator, "cpu_frequency_hz");
        if (cJSON_IsNumber(cpu_frequency_hz)) {
            config->cpu_frequency_hz = cpu_frequency_hz->valueint;
        }
        cJSON* quirk_mode = cJSON_GetObjectItem(emulator, "quirk_mode");
        if (cJSON_IsNumber(quirk_mode)) {
            config->quirk_mode = quirk_mode->valueint;
        }
        cJSON* recently_opened_file = cJSON_GetObjectItem(emulator, "recently_opened_file");
        if (cJSON_IsString(recently_opened_file) && recently_opened_file->valuestring != NULL) {
            strncpy(config->current_rom_path, recently_opened_file->valuestring, sizeof(config->current_rom_path) - 1);
            config->current_rom_path[sizeof(config->current_rom_path) - 1] = '\0';
        }
    }

    if (display) {
        cJSON* color_theme = cJSON_GetObjectItem(display, "color_theme");
        cJSON* color_bg = cJSON_GetObjectItem(display, "current_background_color");
        cJSON* color_fg = cJSON_GetObjectItem(display, "current_foreground_color");
        config->color_bg = hex_str_to_u32(color_bg->valuestring);
        config->color_fg = hex_str_to_u32(color_fg->valuestring);
        cJSON* custom_themes = cJSON_GetObjectItem(display, "custom_themes");
        if (cJSON_IsArray(custom_themes)) {
            int array_size = cJSON_GetArraySize(custom_themes);
            config->custom_theme_count = array_size;
            for (int i = 0; i < array_size && i < 100; i++) {
                cJSON* item = cJSON_GetArrayItem(custom_themes, i);

                cJSON* name_obj = cJSON_GetObjectItem(item, "name");
                if (cJSON_IsString(name_obj)) {
                    strncpy(config->custom_theme_table[i].name, name_obj->valuestring, 31);
                    config->custom_theme_table[i].name[31] = '\0';
                }
                parse_json_color(cJSON_GetObjectItem(item, "background_color"), &config->custom_theme_table[i].bg);
                parse_json_color(cJSON_GetObjectItem(item, "foreground_color"), &config->custom_theme_table[i].fg);
            }
        }
        if (cJSON_IsNumber(color_theme)) {
            if (color_theme->valueint < 100) {
                config->theme_type = color_theme->valueint;
            }
        }
    }

    if (input) {
        cJSON* is_default = cJSON_GetObjectItem(input, "keymap_is_default");
        if (cJSON_IsBool(is_default)) {
            config->keymap_is_default = cJSON_IsTrue(is_default);
        }
        cJSON* hotkeys = cJSON_GetObjectItem(input, "hotkeys");
        if (hotkeys) {
            config->key_pause = SDL_GetKeyFromName(cJSON_GetObjectItem(hotkeys, "pause")->valuestring);
            config->key_restart = SDL_GetKeyFromName(cJSON_GetObjectItem(hotkeys, "restart")->valuestring);
            config->key_fullscreen = SDL_GetKeyFromName(cJSON_GetObjectItem(hotkeys, "fullscreen")->valuestring);
            config->key_minimize = SDL_GetKeyFromName(cJSON_GetObjectItem(hotkeys, "minimize")->valuestring);
            config->key_debug = SDL_GetKeyFromName(cJSON_GetObjectItem(hotkeys, "debug")->valuestring);
        }
        cJSON* keymap_obj = cJSON_GetObjectItem(input, "keymap");
        if (keymap_obj) {
            const char* chip8_hex_chars[]
                = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };
            for (int i = 0; i < 16; i++) {
                cJSON* k = cJSON_GetObjectItem(keymap_obj, chip8_hex_chars[i]);
                if (cJSON_IsString(k)) {
                    config->custom_key_map[i] = SDL_GetKeyFromName(k->valuestring);
                }
            }
        }
    }
    cJSON_Delete(root);
}

static cJSON* struct_to_json(AppConfig* config)
{
    cJSON* root = cJSON_CreateObject();
    if (root == NULL) return root;
    cJSON* emulator = cJSON_AddObjectToObject(root, "emulator");
    cJSON_AddNumberToObject(emulator, "cpu_frequency_hz", config->cpu_frequency_hz);
    cJSON_AddNumberToObject(emulator, "quirk_mode", config->quirk_mode);
    if (strlen(config->current_rom_path) > 0) {
        cJSON_AddStringToObject(emulator, "recently_opened_file", config->current_rom_path);
    }

    cJSON* display = cJSON_AddObjectToObject(root, "display");
    cJSON_AddNumberToObject(display, "color_theme", config->theme_type);

    char bg_str[10];
    char fg_str[10];
    u32_to_hex_str(config->color_bg, bg_str);
    u32_to_hex_str(config->color_fg, fg_str);
    cJSON_AddStringToObject(display, "current_background_color", bg_str);
    cJSON_AddStringToObject(display, "current_foreground_color", fg_str);
    cJSON* custom_themes = cJSON_AddArrayToObject(display, "custom_themes");

    for (int i = 0; i < config->custom_theme_count; i++) {
        cJSON* item = cJSON_CreateObject();

        cJSON_AddStringToObject(item, "name", config->custom_theme_table[i].name);

        char bg[10];
        char fg[10];
        parse_config_color(&config->custom_theme_table[i].bg, bg);
        parse_config_color(&config->custom_theme_table[i].fg, fg);
        cJSON_AddStringToObject(item, "background", bg);
        cJSON_AddStringToObject(item, "foreground", fg);

        cJSON_AddItemToArray(custom_themes, item);
    }

    cJSON* input = cJSON_AddObjectToObject(root, "input");

    cJSON_AddBoolToObject(input, "keymap_is_default", config->keymap_is_default);
    cJSON* hotkeys = cJSON_AddObjectToObject(root, "hotkeys");
    cJSON_AddStringToObject(hotkeys, "pause", SDL_GetKeyName(config->key_pause));
    cJSON_AddStringToObject(hotkeys, "restart", SDL_GetKeyName(config->key_restart));
    cJSON_AddStringToObject(hotkeys, "fullscreen", SDL_GetKeyName(config->key_fullscreen));
    cJSON_AddStringToObject(hotkeys, "minimize", SDL_GetKeyName(config->key_minimize));
    cJSON_AddStringToObject(hotkeys, "debug", SDL_GetKeyName(config->key_debug));

    cJSON* keymap_obj = cJSON_AddObjectToObject(input, "keymap");
    for (int i = 0; i < 16; i++) {
        const char* chip8_hex_chars[]
            = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };
        cJSON_AddStringToObject(keymap_obj, chip8_hex_chars[i], SDL_GetKeyName(config->custom_key_map[i]));
    }
    return root;
}

static void parse_json_color(cJSON* obj, ColorRGBA* out_color)
{
    if (cJSON_IsString(obj) && obj->valuestring != NULL) {
        uint32_t u32_col = hex_str_to_u32((obj->valuestring));

        out_color->r = color_get_r(u32_col);
        out_color->g = color_get_g(u32_col);
        out_color->b = color_get_b(u32_col);
        out_color->a = color_get_a(u32_col);
    }
}
static void parse_config_color(ColorRGBA* in_color, char* hex_str)
{
    uint32_t u32_col = color_to_u32(in_color->r, in_color->g, in_color->b, in_color->a);
    u32_to_hex_str(u32_col, hex_str);
}