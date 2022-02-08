#include "generic_isap.h"
#include <stdio.h>

#define CRYPTO_PLAINBYTES 16

#if defined(GPIO)
    #define OUT_PIN "24"
    #define GPIO_DIRECTION "/sys/class/gpio/gpio" OUT_PIN "/direction"
    #define GPIO_VALUE "/sys/class/gpio/gpio" OUT_PIN "/value"


FILE* setup_trigger() {
    // Export the desired pin by writing to /sys/class/gpio/export
    FILE* fp = fopen("/sys/class/gpio/export", "w");
    if (fp == NULL) {
        perror("Unable to open /sys/class/gpio/export");
        return NULL;
    }
    if (fwrite(OUT_PIN, 1, 2, fp) != 2) {
        perror("Error writing to /sys/class/gpio/export");
        return NULL;
    }
    fclose(fp);

    // Set the pin to be an output by writing "out" to /sys/class/gpio/gpio24/direction
    fp = fopen(GPIO_DIRECTION, "w");
    if (fp == NULL) {
        perror("Unable to open " GPIO_DIRECTION);
        return NULL;
    }
    if (fwrite("out", 1, 3, fp) != 3) {
        perror("Error writing to " GPIO_DIRECTION);
        return NULL;
    }
    fclose(fp);
    fp = fopen(GPIO_VALUE, "w");
    if (fp == NULL) {
        perror("Unable to open " GPIO_VALUE);
        return NULL;
    }
    return fp;
}

int write_trigger(FILE* fp, const char* val) {
    if (fwrite(val, 1, 1, fp) != 1) {
        perror("Error writing to " GPIO_VALUE);
        return -1;
    }
    return 0;
}
#endif

int main(int argc, char const *argv[])
{
    uint8_t nonce[CRYPTO_NPUBBYTES];
    uint8_t key[CRYPTO_KEYBYTES];
    uint8_t plaintext[CRYPTO_PLAINBYTES];
    uint8_t ciphertext[CRYPTO_PLAINBYTES];
    FILE* in_fp = stdin;
    FILE* out_fp = stdout;
    // fixed nonce for now
    for (int i = 0; i < CRYPTO_NPUBBYTES; ++i)
    {
        nonce[i] = i + 0x13371337;
    }
    setvbuf(in_fp, NULL, _IONBF, 0);
    // setup trigger
    #if defined(GPIO)
    FILE* gpio_fp = setup_trigger();
    if (gpio_fp == NULL) {
        return 1;
    }
    #endif
    while (1) {
        // fflush(in_fp);
        if (fread(plaintext, 1, sizeof(plaintext), in_fp) != sizeof(plaintext)) {
            perror("Error reading plaintext");
            return 1;
        }
        // printf("p: %.16s\n", plaintext);
        // fflush(in_fp);
        if (fread(key, 1, sizeof(key), in_fp) != sizeof(key)) {
            perror("Error reading key");
            return 1;
        }
        // printf("k: %.16s\n", key);\
        // raise trigger
        #if defined(GPIO)
        write_trigger(gpio_fp, "1");
        #endif
        isap_enc(key, nonce, plaintext, sizeof(plaintext), ciphertext);
        // lower trigger
        #if defined(GPIO)
        write_trigger(gpio_fp, "0");
        #endif
        if (fwrite(ciphertext, 1, sizeof(ciphertext), out_fp) != sizeof(ciphertext)) {
            perror("Error writing out");
            return 1;
        }
        fprintf(out_fp, "\n");
    }
    return 0;
}