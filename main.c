#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <cyaml/cyaml.h>
#include <wiringPi.h>
#include <softPwm.h>

typedef struct
{
    int channel;
    int temp_fan_start;
    int temp_fan_max;
    int temp_fan_stop;
    int pwm_start;
    int pwm_max;
    int delay;
} FanConfig;

static const cyaml_schema_field_t top_mapping_schema[] = {
    CYAML_FIELD_INT("channel", CYAML_FLAG_DEFAULT,
                    FanConfig, channel),
    CYAML_FIELD_INT("temp_fan_start", CYAML_FLAG_DEFAULT,
                    FanConfig, temp_fan_start),
    CYAML_FIELD_INT("temp_fan_max", CYAML_FLAG_DEFAULT,
                    FanConfig, temp_fan_max),
    CYAML_FIELD_INT("temp_fan_stop", CYAML_FLAG_DEFAULT,
                    FanConfig, temp_fan_stop),
    CYAML_FIELD_INT("pwm_start", CYAML_FLAG_DEFAULT,
                    FanConfig, pwm_start),
    CYAML_FIELD_INT("pwm_max", CYAML_FLAG_DEFAULT,
                    FanConfig, pwm_max),
    CYAML_FIELD_INT("delay", CYAML_FLAG_DEFAULT,
                    FanConfig, delay),
    CYAML_FIELD_END};

static const cyaml_schema_value_t top_schema = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
                        FanConfig, top_mapping_schema),
};

static const cyaml_config_t config = {
    .log_fn = cyaml_log,
    .mem_fn = cyaml_mem,
    .log_level = CYAML_LOG_WARNING,
};

static float cpu_temp(void)
{
    char buffer[16];
    FILE *file = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error reading tempture\n");
        exit(1);
    }
    fgets(buffer, sizeof(buffer), file);
    fclose(file);
    return (float)atof(buffer) / 1000;
}

static void set_fan_pwm(int pwm, int channel)
{
    softPwmWrite(channel, pwm);
}

static int calculate_pwm(float temp, int temp_fan_start, int temp_fan_max, int pwm_start, int pwm_max)
{
    float pwm_float = ((temp - (float)temp_fan_start) / (float)(temp_fan_max - temp_fan_start)) * (float)(pwm_max - pwm_start) + (float)pwm_start;
    int pwm = (int)round(pwm_float);
    return (pwm < pwm_start) ? pwm_start : ((pwm > pwm_max) ? pwm_max : pwm);
}

static int check_fan_config(FanConfig *fc)
{
    int n_err = 0;
    if (!((fc->temp_fan_stop <= fc->temp_fan_start) && (fc->temp_fan_start <= fc->temp_fan_max)))
    {
        fprintf(stderr, "Config required: temp_fan_stop <= temp_fan_start <= temp_fan_max");
        n_err++;
    }
    if (!((0 < fc->pwm_start) && (fc->pwm_start <= fc->pwm_max) && (fc->pwm_max <= 100)))
    {
        fprintf(stderr, "Config required: 0 < pwm_start <= pwm_max <= 100");
        n_err++;
    }
    return n_err;
}

int main(int argc, char *argv[])
{
    cyaml_err_t err;
    FanConfig *fc = NULL;
    enum
    {
        ARG_PROG_NAME,
        ARG_PATH_IN,
        ARG__COUNT,
    };

    int pwm = 0;
    float temp = cpu_temp();

    setvbuf(stdout, NULL, _IOLBF, 0);

    if (argc != ARG__COUNT)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s config.yaml\n", argv[ARG_PROG_NAME]);
        return EXIT_FAILURE;
    }

    err = cyaml_load_file(argv[ARG_PATH_IN], &config,
                          &top_schema, (cyaml_data_t **)&fc, NULL);
    if (err != CYAML_OK)
    {
        fprintf(stderr, "YAML CONFIG ERROR: %s\n", cyaml_strerror(err));
        return EXIT_FAILURE;
    }

    fprintf(stdout, "FanConfig loaded -> ");
    fprintf(stdout, "channel: %d, ", fc->channel);
    fprintf(stdout, "temp_fan_start: %d, ", fc->temp_fan_start);
    fprintf(stdout, "temp_fan_max: %d, ", fc->temp_fan_max);
    fprintf(stdout, "temp_fan_stop: %d, ", fc->temp_fan_stop);
    fprintf(stdout, "pwm_start: %d, ", fc->pwm_start);
    fprintf(stdout, "pwm_max: %d, ", fc->pwm_max);
    fprintf(stdout, "delay: %d\n", fc->delay);

    if (check_fan_config(fc) > 0)
    {
        return 1;
    }

    if (wiringPiSetup() == -1)
    {
        fprintf(stderr, "Unable to initialize WiringPi\n");
        return 1;
    }
    softPwmCreate(fc->channel, pwm, 100);
    fprintf(stdout, "Fan init -> [temp: %.3f, pwm: %d]\n", temp, pwm);
    delay(1000);

    while (1)
    {
        temp = cpu_temp();

        if (temp > fc->temp_fan_start)
        {
            pwm = calculate_pwm(temp,
                                fc->temp_fan_start, fc->temp_fan_max,
                                fc->pwm_start, fc->pwm_max);
            set_fan_pwm(pwm, fc->channel);
            fprintf(stdout, "Fan up -> temp: %.3f, pwm: %d\n", temp, pwm);
        }
        else if (pwm > 0)
        {
            if (temp > fc->temp_fan_stop)
            {
                pwm = fc->pwm_start;
                set_fan_pwm(pwm, fc->channel);
                fprintf(stdout, "Fan wait -> temp: %.3f, pwm: %d\n", temp, pwm);
            }
            else
            {
                pwm = 0;
                set_fan_pwm(pwm, fc->channel);
                fprintf(stdout, "Fan down -> temp: %.3f, pwm: %d\n", temp, pwm);
            }
        }

        delay((unsigned int)fc->delay);
    }

    cyaml_free(&config, &top_schema, fc, 0);

    return EXIT_SUCCESS;
}
