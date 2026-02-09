#include "bmctool/ipmi_context.h"
#include "bmctool/ipmi_commands.h"
#include "bmctool/redfish.h"
#include <stdio.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char* prog) {
    printf("Usage: %s [options] <protocol> <command>\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -H, --host <host>      BMC hostname or IP\n");
    printf("  -p, --port <port>      BMC port (IPMI: 623, Redfish: 443)\n");
    printf("  -U, --user <user>      Username (Redfish)\n");
    printf("  -P, --password <pass>  Password (Redfish)\n");
    printf("  -f, --format <fmt>     Output format: normal, json, table\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -h, --help             Show this help\n");
    printf("\n");
    printf("Protocols:\n");
    printf("  ipmi                   Use IPMI protocol\n");
    printf("  redfish                Use Redfish protocol\n");
    printf("\n");
    printf("IPMI Commands:\n");
    printf("  get-device-id          Get BMC device information\n");
    printf("  chassis-status         Get chassis power status\n");
    printf("\n");
    printf("Redfish Commands:\n");
    printf("  system <id>            Get system information\n");
    printf("  thermal <id>           Get thermal information\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -H 192.168.1.100 ipmi get-device-id\n", prog);
    printf("  %s -H https://bmc.local -U admin -P pwd redfish system 1\n", prog);
    printf("  %s -H 192.168.1.100 -f table ipmi chassis-status\n", prog);
}

static void print_manufacturer(uint32_t mfg_id) {
    switch (mfg_id) {
        case 0x000157: printf("Intel"); break;
        case 0x0002A2: printf("IBM"); break;
        case 0x00000B: printf("HP"); break;
        case 0x0019A9: printf("Dell"); break;
        case 0x00004D: printf("Fujitsu"); break;
        case 0x0000B4: printf("Advantech"); break;
        default: printf("Unknown (0x%06x)", mfg_id);
    }
}

static int cmd_ipmi_get_device_id(ipmi_ctx_t* ctx) {
    ipmi_device_id_t device_id;
    
    int ret = ipmi_cmd_get_device_id(ctx, &device_id);
    if (ret != BMC_SUCCESS) {
        fprintf(stderr, "Error: %s\n", bmc_error_str(ret));
        return 1;
    }
    
    if (g_output_format == OUTPUT_FORMAT_TABLE) {
        // 表格模式
        const char* headers[] = {"Property", "Value"};
        table_init(2, headers);
        table_print_header();
        
        char temp[128];
        const char* row[2];
        
        row[0] = "Device ID";
        snprintf(temp, sizeof(temp), "0x%02x", device_id.device_id);
        row[1] = temp;
        table_print_row(row);
        
        row[0] = "Firmware Version";
        snprintf(temp, sizeof(temp), "%d.%d", device_id.firmware_rev1, device_id.firmware_rev2);
        row[1] = temp;
        table_print_row(row);
        
        row[0] = "IPMI Version";
        snprintf(temp, sizeof(temp), "%d.%d", 
                device_id.ipmi_version & 0x0F, (device_id.ipmi_version >> 4) & 0x0F);
        row[1] = temp;
        table_print_row(row);
        
        table_print_footer();
    } else {
        // 一般模式（美化版）
        print_section_header("BMC Device Information");
        
        char temp[128];
        
        snprintf(temp, sizeof(temp), "0x%02x", device_id.device_id);
        print_kv("Device ID", temp);
        
        snprintf(temp, sizeof(temp), "%d", device_id.device_revision);
        print_kv("Device Revision", temp);
        
        snprintf(temp, sizeof(temp), "%d.%d", 
                device_id.firmware_rev1, device_id.firmware_rev2);
        print_kv("Firmware Version", temp);
        
        snprintf(temp, sizeof(temp), "%d.%d",
                device_id.ipmi_version & 0x0F, 
                (device_id.ipmi_version >> 4) & 0x0F);
        print_kv("IPMI Version", temp);
        
        uint32_t mfg = device_id.manufacturer_id[0] |
                       (device_id.manufacturer_id[1] << 8) |
                       (device_id.manufacturer_id[2] << 16);
        snprintf(temp, sizeof(temp), "0x%06x (", mfg);
        print_kv("Manufacturer ID", temp);
        printf("%*s   ", 20, "");
        print_manufacturer(mfg);
        printf(")\n");
        
        uint16_t prod = device_id.product_id[0] | 
                        (device_id.product_id[1] << 8);
        snprintf(temp, sizeof(temp), "0x%04x", prod);
        print_kv("Product ID", temp);
    }
    
    return 0;
}

static int cmd_ipmi_chassis_status(ipmi_ctx_t* ctx) {
    ipmi_chassis_status_t status;
    
    int ret = ipmi_cmd_get_chassis_status(ctx, &status);
    if (ret != BMC_SUCCESS) {
        fprintf(stderr, "Error: %s\n", bmc_error_str(ret));
        return 1;
    }
    
    print_section_header("Chassis Status");
    
    printf("\nCurrent Power State:\n");
    print_kv("  Power", (status.current_power_state & 0x01) ? "ON" : "OFF");
    print_kv("  Power overload", (status.current_power_state & 0x02) ? "Yes" : "No");
    print_kv("  Interlock", (status.current_power_state & 0x04) ? "Active" : "Inactive");
    print_kv("  Power fault", (status.current_power_state & 0x08) ? "Detected" : "No");
    print_kv("  Power control fault", (status.current_power_state & 0x10) ? "Detected" : "No");
    
    char temp[32];
    snprintf(temp, sizeof(temp), "0x%02x", status.last_power_event);
    printf("\n");
    print_kv("Last Power Event", temp);
    
    snprintf(temp, sizeof(temp), "0x%02x", status.misc_chassis_state);
    print_kv("Misc Chassis State", temp);
    
    return 0;
}

static int cmd_redfish_system(redfish_ctx_t* ctx, const char* system_id) {
    redfish_system_t system;
    memset(&system, 0, sizeof(system));
    
    int ret = redfish_get_system(ctx, system_id, &system);
    if (ret != BMC_SUCCESS) {
        fprintf(stderr, "Error: %s\n", bmc_error_str(ret));
        return 1;
    }
    
    print_section_header("System Information");
    
    print_kv("System ID", system.id);
    print_kv("Name", system.name);
    print_kv("Manufacturer", system.manufacturer);
    print_kv("Model", system.model);
    print_kv("Serial Number", system.serial_number);
    print_kv("Power State", system.power_state);
    if (system.bios_version[0]) {
        print_kv("BIOS Version", system.bios_version);
    }
    
    return 0;
}

static int cmd_redfish_thermal(redfish_ctx_t* ctx, const char* chassis_id) {
    print_section_header("Thermal Information");
    printf("Chassis: %s\n\n", chassis_id);
    
    int ret = redfish_get_thermal(ctx, chassis_id);
    if (ret != BMC_SUCCESS) {
        fprintf(stderr, "Error: %s\n", bmc_error_str(ret));
        return 1;
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    const char* host = NULL;
    uint16_t port = 0;
    const char* username = NULL;
    const char* password = NULL;
    int verbose = 0;
    const char* format = "normal";
    
    static struct option long_options[] = {
        {"host",     required_argument, 0, 'H'},
        {"port",     required_argument, 0, 'p'},
        {"user",     required_argument, 0, 'U'},
        {"password", required_argument, 0, 'P'},
        {"format",   required_argument, 0, 'f'},
        {"verbose",  no_argument,       0, 'v'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "H:p:U:P:f:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'H':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'U':
                username = optarg;
                break;
            case 'P':
                password = optarg;
                break;
            case 'f':
                format = optarg;
                if (strcmp(format, "json") == 0) {
                    g_output_format = OUTPUT_FORMAT_JSON;
                } else if (strcmp(format, "table") == 0) {
                    g_output_format = OUTPUT_FORMAT_TABLE;
                } else {
                    g_output_format = OUTPUT_FORMAT_NORMAL;
                }
                break;
            case 'v':
                verbose = 1;
                g_log_level = LOG_LEVEL_DEBUG;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (!host) {
        fprintf(stderr, "Error: Host required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (optind >= argc) {
        fprintf(stderr, "Error: Protocol required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    const char* protocol = argv[optind];
    
    if (strcmp(protocol, "ipmi") == 0) {
        if (optind + 1 >= argc) {
            fprintf(stderr, "Error: IPMI command required\n\n");
            print_usage(argv[0]);
            return 1;
        }
        
        const char* cmd = argv[optind + 1];
        
        if (port == 0) {
            port = IPMI_DEFAULT_PORT;
        }
        
        ipmi_ctx_t* ctx = ipmi_ctx_create();
        if (!ctx) {
            fprintf(stderr, "Error: Failed to create IPMI context\n");
            return 1;
        }
        
        ipmi_ctx_set_target(ctx, host, port);
        
        if (ipmi_ctx_open(ctx) != BMC_SUCCESS) {
            fprintf(stderr, "Error: Failed to connect to %s:%d\n", host, port);
            ipmi_ctx_destroy(ctx);
            return 1;
        }
        
        int ret = 0;
        if (strcmp(cmd, "get-device-id") == 0) {
            ret = cmd_ipmi_get_device_id(ctx);
        } else if (strcmp(cmd, "chassis-status") == 0) {
            ret = cmd_ipmi_chassis_status(ctx);
        } else {
            fprintf(stderr, "Error: Unknown IPMI command '%s'\n", cmd);
            ret = 1;
        }
        
        ipmi_ctx_close(ctx);
        ipmi_ctx_destroy(ctx);
        return ret;
        
    } else if (strcmp(protocol, "redfish") == 0) {
        if (optind + 1 >= argc) {
            fprintf(stderr, "Error: Redfish command required\n\n");
            print_usage(argv[0]);
            return 1;
        }
        
        const char* cmd = argv[optind + 1];
        
        redfish_ctx_t* ctx = redfish_ctx_create();
        if (!ctx) {
            fprintf(stderr, "Error: Failed to create Redfish context\n");
            return 1;
        }
        
        redfish_ctx_set_endpoint(ctx, host);
        
        if (username && password) {
            redfish_ctx_set_auth(ctx, username, password);
        }
        
        int ret = 0;
        if (strcmp(cmd, "system") == 0) {
            if (optind + 2 >= argc) {
                fprintf(stderr, "Error: System ID required\n");
                ret = 1;
            } else {
                ret = cmd_redfish_system(ctx, argv[optind + 2]);
            }
        } else if (strcmp(cmd, "thermal") == 0) {
            if (optind + 2 >= argc) {
                fprintf(stderr, "Error: Chassis ID required\n");
                ret = 1;
            } else {
                ret = cmd_redfish_thermal(ctx, argv[optind + 2]);
            }
        } else {
            fprintf(stderr, "Error: Unknown Redfish command '%s'\n", cmd);
            ret = 1;
        }
        
        redfish_ctx_destroy(ctx);
        return ret;
        
    } else {
        fprintf(stderr, "Error: Unknown protocol '%s'\n", protocol);
        print_usage(argv[0]);
        return 1;
    }
}
