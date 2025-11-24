#include <config.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ConfigurationFile game_find_config(const char* filename)
{
  ConfigurationFile result = {
      .handle = NULL,
      .filename = NULL,
      .err_code = SUCCESS,
  };

  char full_path[1'024] = {0};
  char* env_val;

  // Try LOCALAPPDATA
  if ((env_val = getenv("LOCALAPPDATA")) != NULL && *env_val != '\0')
  {
    snprintf(full_path, 1'024, "%s\\HoloTable\\%s", env_val, filename);
    result.handle = fopen(full_path, "r");
  }

  // Try USERPROFILE if not found yet
  if (result.handle == NULL && (env_val = getenv("USERPROFILE")) != NULL && *env_val != '\0')
  {
    snprintf(full_path, 1'024, "%s\\AppData\\Local\\HoloTable\\%s", env_val, filename);
    result.handle = fopen(full_path, "r");
  }

  // Try XDG_DATA_HOME if not found yet
  if (result.handle == NULL && (env_val = getenv("XDG_DATA_HOME")) != NULL && *env_val != '\0')
  {
    snprintf(full_path, 1'024, "%s/HoloTable/%s", env_val, filename);
    result.handle = fopen(full_path, "r");
  }

  // Try HOME if not found yet
  if (result.handle == NULL && (env_val = getenv("HOME")) != NULL && *env_val != '\0')
  {
    snprintf(full_path, 1'024, "%s/.local/share/HoloTable/%s", env_val, filename);
    result.handle = fopen(full_path, "r");
  }

  // Try local assets folder as a fallback
  if (result.handle == NULL)
  {
    snprintf(full_path, 1'024, "assets/%s", filename);
    result.handle = fopen(full_path, "r");
  }
  if (result.handle == NULL)
  {
    snprintf(full_path, 1'024, "assets\\%s", filename);
    result.handle = fopen(full_path, "r");
  }

  if (result.handle == NULL)
  {
    if (errno == ENOENT)
    {
      result.err_code = NOT_FOUND;
    }
    else if (errno == EACCES)
    {
      result.err_code = PERMISSION_DENIED;
    }
    else
    {
      result.err_code = UNHANDLED_ERR;
    }
  }
  else
  {
    result.filename = strdup(full_path);
    if (result.filename == NULL)
    {
      fprintf(stderr, "Memory failure! FATAL!\n");
      fclose(result.handle);
      abort();
    }
    result.err_code = SUCCESS;
  }

  return result;
}

void __trim_str(char* str)
{
  if (!str || *str == '\0')
  {
    return; // Handle empty or NULL strings
  }

  int end = strlen(str) - 1;
  int begin = 0;

  while (begin <= end && isspace((unsigned char)str[begin]))
  {
    begin += 1;
  }

  while (end >= begin && isspace((unsigned char)str[end]))
  {
    end -= 1;
  }

  int i;
  for (i = begin; i <= end; i += 1)
  {
    str[i - begin] = str[i];
  }

  str[i - begin] = '\0';
}

WindowDimensions game_parse_display_size(ConfigurationFile cfg)
{
  int width = 800;
  int height = 600;
  if (cfg.err_code != SUCCESS)
  {
    return (WindowDimensions){width, height};
  }

  char line[100];
  bool in_display = false;

  while (fgets(line, sizeof(line), cfg.handle))
  {
    char* pos = strchr(line, '#');
    if (pos)
      *pos = '\0'; // truncate line at #

    __trim_str(line);

    if (strlen(line) == 0)
      continue;

    if (line[0] == '[' && line[strlen(line) - 1] == ']')
    {
      in_display = (strcmp(line, "[Display]") == 0);
      continue;
    }
    if (!in_display)
      continue;

    char* eq = strchr(line, '=');
    if (!eq)
      continue;

    *eq = '\0';
    char* key_ptr = line;
    char* val_ptr = eq + 1; // Start of the value string

    __trim_str(key_ptr);
    __trim_str(val_ptr);

    size_t val_len = strlen(val_ptr);
    if (val_len >= 2)
    {
      char first = val_ptr[0];
      char last = val_ptr[val_len - 1];

      if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
      {
        memmove(val_ptr, val_ptr + 1, val_len - 2);

        val_ptr[val_len - 2] = '\0';

        __trim_str(val_ptr);
      }
    }

    if (strcmp(key_ptr, "width") == 0 || strcmp(key_ptr, "height") == 0)
    {
      char* endptr;
      long parsed_value_long = strtol(val_ptr, &endptr, 10); // Base 10 conversion

      if (*val_ptr != '\0' && *endptr == '\0')
      {
        int parsed_value = (int)parsed_value_long; // Cast to int (assuming range is fine)

        if (strcmp(key_ptr, "width") == 0)
          width = parsed_value;
        else
          height = parsed_value;
      }
    }
  }

  return (WindowDimensions){width, height};
}
