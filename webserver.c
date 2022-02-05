#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

#include "tusb_lwip_glue.h"

#define LED_PIN     25

// let our webserver do some dynamic handling
static const char *cgi_toggle_led(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    gpio_put(LED_PIN, !gpio_get(LED_PIN));
    return "/index.shtml";
}

static const char *cgi_reset_usb_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    reset_usb_boot(0, 0);
    return "/index.shtml";
}

static const tCGI cgi_handlers[] = {
  {
    "/toggle_led",
    cgi_toggle_led
  },
  {
    "/reset_usb_boot",
    cgi_reset_usb_boot
  }
};

u16_t adc_ssi_handler(int iIndex, char *buf, int buflen, u16_t current_tag_number, u16_t *next_tag_number)
{
   static char counter=0;
   // ...put up to buflen bytes in buf and return number of bytes put there
   for (int i=0; i<buflen; i++) {
       buf[i] = counter++;   // perhaps i + iIndex ?
   }
//   if ( current_tag_number < 100  ) {
//   should go forever, but it stops at 12'582'721 bytes
//   which are transferred at 136 kB / s (a bit slower than needed for full ADC speed)
       *next_tag_number = current_tag_number + 1;
//   }
   return buflen;
}

const char * ssi_tags[] = {
	   "adc"
};


int main()
{
    // Initialize tinyusb, lwip, dhcpd and httpd
    init_lwip();
    wait_for_netif_is_up();
    dhcpd_init();
    httpd_init();
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
    http_set_ssi_handler(adc_ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
    
    // For toggle_led
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true)
    {
        tud_task();
        service_traffic();
    }

    return 0;
}
