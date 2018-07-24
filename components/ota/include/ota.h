#ifndef _OTA_H_
#define _OTA_H_

#include "esp_ota_ops.h"

#define BUFFSIZE 1024
#define TEXT_BUFFSIZE 1024

int read_until(char *buffer, char delim, int len);

bool read_past_http_header(char text[], int total_len, esp_ota_handle_t update_handle);

bool connect_to_http_server();

void task_fatal_error();

int ota_run(char *server, char *port, char *filename);

#endif
