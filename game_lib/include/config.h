#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H
#include <stddef.h>
#include <stdio.h>

typedef enum
{
  SUCCESS = 0,
  NOT_FOUND = 1,
  PERMISSION_DENIED = 2,
  INVALID_FORMAT = 3,
  UNHANDLED_ERR = 4,
} ConfigurationError;

typedef struct
{
  char* filename;
  FILE* handle;
  ConfigurationError err_code;
} ConfigurationFile;

typedef struct
{
  size_t width;
  size_t height;
} WindowDimensions;

ConfigurationFile game_find_config(const char* filename);
WindowDimensions game_parse_display_size(ConfigurationFile cfg);
#endif // GAME_CONFIG_H
