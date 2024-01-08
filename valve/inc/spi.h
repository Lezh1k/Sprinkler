#include <stdint.h>

void spi_init(void);

void spi_send_str_async(const char *str,
                        uint8_t len,
                        void (*cb_transmission_finished)(void));
