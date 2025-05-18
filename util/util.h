int draw_box(int top);
int pretty_print_title(char* content);
int pretty_print(const char* content, const char* content2);
int print_progress(char* name, float value, float max_value);

int get_mib(long long value);
int get_gib(long long value);

#if defined(__OpenBSD__)
    int microkelvin_to_celsius(long int value);

    struct sensor get_sensor_openbsd(char* sensor_name, int sensor_type);
#endif
