void draw_box(int top);
void print_title(char *content);
void pretty_print(char *content, char *content2);
void print_progress(char *name, float value, float max_value);

int get_mib(long long value);
int get_gib(long long value);

#if defined(__OpenBSD__)
    int microkelvin_to_celsius(long int value);

    struct sensor get_sensor_openbsd(char *sensor_name, int sensor_type);
#endif
