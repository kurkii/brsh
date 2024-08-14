#define BRSH_KEY_PATH_INDEX 0

#define KEY_LENGTH 256
#define VALUE_LENGTH 256
#define VALUE_STRING_SIZE 256
#define VALUE_BOOL_SIZE 6

#define BRSH_LINE_MAX 2048

#define CONFIG_FILENAME ".brsh_config"

int keyname_to_index(char *name);
int write_config(char* name, char* value);
int read_config(void);
void config_initial_setup();
void get_config_path();
