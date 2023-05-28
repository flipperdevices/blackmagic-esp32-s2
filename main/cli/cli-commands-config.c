#include "cli.h"
#include "cli-args.h"
#include "cli-commands.h"
#include "helpers.h"
#include "nvs-config.h"
#include <nvs_flash.h>
#include <string.h>

void cli_config_get(Cli* cli, mstring_t* args) {
    mstring_t* value = mstring_alloc();
    WiFiMode wifi_mode;
    UsbMode usb_mode;

    nvs_config_get_ap_ssid(value);
    cli_printf(cli, "ap_ssid: %s", mstring_get_cstr(value));
    cli_write_eol(cli);

    nvs_config_get_ap_pass(value);
    cli_printf(cli, "ap_pass: %s", mstring_get_cstr(value));
    cli_write_eol(cli);

    nvs_config_get_sta_ssid(value);
    cli_printf(cli, "sta_ssid: %s", mstring_get_cstr(value));
    cli_write_eol(cli);

    nvs_config_get_sta_pass(value);
    cli_printf(cli, "sta_pass: %s", mstring_get_cstr(value));
    cli_write_eol(cli);

    nvs_config_get_hostname(value);
    cli_printf(cli, "hostname: %s", mstring_get_cstr(value));
    cli_write_eol(cli);

    nvs_config_get_wifi_mode(&wifi_mode);
    switch(wifi_mode) {
    case WiFiModeAP:
        mstring_set(value, CFG_WIFI_MODE_AP);
        break;
    case WiFiModeSTA:
        mstring_set(value, CFG_WIFI_MODE_STA);
        break;
    case WiFiModeDisabled:
        mstring_set(value, CFG_WIFI_MODE_DISABLED);
        break;
    }

    cli_printf(cli, "wifi_mode: %s", mstring_get_cstr(value));
    cli_write_eol(cli);

    nvs_config_get_usb_mode(&usb_mode);
    switch(usb_mode) {
    case UsbModeBM:
        mstring_set(value, CFG_USB_MODE_BM);
        break;
    case UsbModeDAP:
        mstring_set(value, CFG_USB_MODE_DAP);
        break;
    }

    cli_printf(cli, "usb_mode: %s", mstring_get_cstr(value));

    mstring_free(value);
}

static void cli_config_set_wifi_mode_usage(Cli* cli) {
    cli_write_str(
        cli,
        "config_set_wifi_mode"
        " <" CFG_WIFI_MODE_AP "|" CFG_WIFI_MODE_STA "|" CFG_WIFI_MODE_DISABLED ">");
    cli_write_eol(cli);
    cli_write_str(cli, " " CFG_WIFI_MODE_AP " (make own WiFi AP)");
    cli_write_eol(cli);
    cli_write_str(cli, " " CFG_WIFI_MODE_STA " (connect to WiFi)");
    cli_write_eol(cli);
    cli_write_str(cli, " " CFG_WIFI_MODE_DISABLED " (disable WiFi)");
    cli_write_eol(cli);
}

void cli_config_set_wifi_mode(Cli* cli, mstring_t* args) {
    mstring_t* mode = mstring_alloc();
    WiFiMode wifi_mode;

    do {
        if(!cli_args_read_string_and_trim(args, mode)) {
            cli_config_set_wifi_mode_usage(cli);
            break;
        }

        if(mstring_cmp_cstr(mode, CFG_WIFI_MODE_AP) == 0) {
            wifi_mode = WiFiModeAP;
        } else if(mstring_cmp_cstr(mode, CFG_WIFI_MODE_STA) == 0) {
            wifi_mode = WiFiModeSTA;
        } else if(mstring_cmp_cstr(mode, CFG_WIFI_MODE_DISABLED) == 0) {
            wifi_mode = WiFiModeDisabled;
        } else {
            cli_config_set_wifi_mode_usage(cli);
            break;
        }

        if(nvs_config_set_wifi_mode(wifi_mode) == ESP_OK) {
            cli_write_str(cli, "OK");
            cli_write_eol(cli);
            cli_write_str(cli, "Reboot to apply");
        } else {
            cli_write_str(cli, "ERR");
        }
    } while(false);

    mstring_free(mode);
}

static void cli_config_set_usb_mode_usage(Cli* cli) {
    cli_write_str(cli, "config_set_usb_mode <" CFG_USB_MODE_BM "|" CFG_USB_MODE_DAP ">");
    cli_write_eol(cli);
    cli_write_str(cli, " " CFG_USB_MODE_BM " (Black Magic Probe mode)");
    cli_write_eol(cli);
    cli_write_str(cli, " " CFG_USB_MODE_DAP " (DAPLink mode)");
    cli_write_eol(cli);
}

void cli_config_set_usb_mode(Cli* cli, mstring_t* args) {
    mstring_t* mode = mstring_alloc();
    UsbMode usb_mode;

    do {
        if(!cli_args_read_string_and_trim(args, mode)) {
            cli_config_set_usb_mode_usage(cli);
            break;
        }

        if(mstring_cmp_cstr(mode, CFG_USB_MODE_BM) == 0) {
            usb_mode = UsbModeBM;
        } else if(mstring_cmp_cstr(mode, CFG_USB_MODE_DAP) == 0) {
            usb_mode = UsbModeDAP;
        } else {
            cli_config_set_usb_mode_usage(cli);
            break;
        }

        if(nvs_config_set_usb_mode(usb_mode) == ESP_OK) {
            cli_write_str(cli, "OK");
            cli_write_eol(cli);
            cli_write_str(cli, "Reboot to apply");
        } else {
            cli_write_str(cli, "ERR");
        }
    } while(false);

    mstring_free(mode);
}

void cli_config_set_ap_pass(Cli* cli, mstring_t* args) {
    mstring_t* pass = mstring_alloc();

    do {
        if(!cli_args_read_quoted_string_and_trim(args, pass)) {
            cli_write_str(cli, "config_set_ap_pass \"<pass>\", min 8 symbols or empty quotes");
            break;
        }

        if(nvs_config_set_ap_pass(pass) == ESP_OK) {
            cli_write_str(cli, "OK");
            cli_write_eol(cli);
            cli_write_str(cli, "Reboot to apply");
        } else {
            cli_write_str(cli, "config_set_ap_pass \"<pass>\", min 8 symbols or empty quotes");
        }
    } while(false);

    mstring_free(pass);
}

void cli_config_set_ap_ssid(Cli* cli, mstring_t* args) {
    mstring_t* ssid = mstring_alloc();

    do {
        if(!cli_args_read_quoted_string_and_trim(args, ssid)) {
            cli_write_str(cli, "config_set_ap_ssid \"<ssid>\", 1-32 symbols");
            break;
        }

        if(nvs_config_set_ap_ssid(ssid) == ESP_OK) {
            cli_write_str(cli, "OK");
            cli_write_eol(cli);
            cli_write_str(cli, "Reboot to apply");
        } else {
            cli_write_str(cli, "config_set_ap_ssid \"<ssid>\", 1-32 symbols");
        }
    } while(false);

    mstring_free(ssid);
}

void cli_config_set_sta_pass(Cli* cli, mstring_t* args) {
    mstring_t* pass = mstring_alloc();

    do {
        if(!cli_args_read_quoted_string_and_trim(args, pass)) {
            cli_write_str(cli, "config_set_sta_pass \"<pass>\", min 8 symbols or empty quotes");
            break;
        }

        if(nvs_config_set_sta_pass(pass) == ESP_OK) {
            cli_write_str(cli, "OK");
            cli_write_eol(cli);
            cli_write_str(cli, "Reboot to apply");
        } else {
            cli_write_str(cli, "config_set_sta_pass \"<pass>\", min 8 symbols or empty quotes");
        }
    } while(false);

    mstring_free(pass);
}

void cli_config_set_sta_ssid(Cli* cli, mstring_t* args) {
    mstring_t* ssid = mstring_alloc();

    do {
        if(!cli_args_read_quoted_string_and_trim(args, ssid)) {
            cli_write_str(cli, "config_set_sta_ssid \"<ssid>\", 1-32 symbols");
            break;
        }

        if(nvs_config_set_sta_ssid(ssid) == ESP_OK) {
            cli_write_str(cli, "OK");
            cli_write_eol(cli);
            cli_write_str(cli, "Reboot to apply");
        } else {
            cli_write_str(cli, "config_set_sta_ssid \"<ssid>\", 1-32 symbols");
        }
    } while(false);

    mstring_free(ssid);
}

void cli_config_set_hostname(Cli* cli, mstring_t* args) {
    mstring_t* hostname = mstring_alloc();

    do {
        if(!cli_args_read_quoted_string_and_trim(args, hostname)) {
            cli_write_str(cli, "config_set_hostname \"<hostname>\", 1-32 symbols");
            break;
        }

        if(nvs_config_set_hostname(hostname) == ESP_OK) {
            cli_write_str(cli, "OK");
            cli_write_eol(cli);
            cli_write_str(cli, "Reboot to apply");
        } else {
            cli_write_str(cli, "config_set_hostname \"<hostname>\", 1-32 symbols");
        }
    } while(false);

    mstring_free(hostname);
}

const char* nvs_type_to_str(nvs_type_t type) {
    switch(type) {
    case NVS_TYPE_U8:
        return "U8";
    case NVS_TYPE_I8:
        return "I8";
    case NVS_TYPE_U16:
        return "U16";
    case NVS_TYPE_I16:
        return "I16";
    case NVS_TYPE_U32:
        return "U32";
    case NVS_TYPE_I32:
        return "I32";
    case NVS_TYPE_U64:
        return "U64";
    case NVS_TYPE_I64:
        return "I64";
    case NVS_TYPE_STR:
        return "STR";
    case NVS_TYPE_BLOB:
        return "BLOB";
    case NVS_TYPE_ANY:
        return "ANY";
    default:
        return "UNK";
    }
}

void cli_nvs_dump(Cli* cli, mstring_t* args) {
    const char* partitions[] = {
        "nvs",
        "nvs_storage",
    };

    for(size_t i = 0; i < COUNT_OF(partitions); i++) {
        cli_write_str(cli, partitions[i]);
        cli_write_str(cli, ":");
        cli_write_eol(cli);

        do {
            nvs_iterator_t it;
            if(nvs_entry_find(partitions[i], NULL, NVS_TYPE_ANY, &it) != ESP_OK) {
                cli_write_str(cli, "ERR");
                cli_write_eol(cli);
                break;
            }
            while(it != NULL) {
                nvs_entry_info_t info;
                nvs_entry_info(it, &info);
                it = nvs_entry_next(it);

                if(strlen(info.namespace_name)) {
                    cli_printf(cli, "  \"%s:%s\"", info.namespace_name, info.key);
                } else {
                    cli_printf(cli, "  \"%s\"", info.key);
                }
                cli_printf(cli, " %s", nvs_type_to_str(info.type));
                cli_write_eol(cli);
            };

        } while(false);
    }

    cli_write_str(cli, "OK");
}
