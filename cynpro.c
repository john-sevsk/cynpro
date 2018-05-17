#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <libusb.h>
#include <stdbool.h>
#include <getopt.h>

#define USB_VID 0x1532
#define USB_PID 0x020d
#define USB_IFACE 2

enum
{
  CMD_COLOR_NONE,
  CMD_COLOR_GREEN,
  CMD_COLOR_BLUE,
  CMD_COLOR_CYAN
};

enum
{
  CMD_LED_NONE,
  CMD_LED_ON,
  CMD_LED_OFF
};

enum
{
  CMD_BREATHING_NONE,
  CMD_BREATHING_ON,
  CMD_BREATHING_OFF,
  CMD_BREATHING_CYCLING
};

struct rgb {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  bool isSet;
};

int device_found = 0;
int cmd_color = CMD_COLOR_NONE;
int cmd_led = CMD_LED_NONE;
int cmd_breathing = CMD_BREATHING_NONE;
long cmd_led_intensity = -1;

int option_index = 0;
static struct rgb rgb_values = {
  .red = 0,
  .green = 0,
  .blue = 0,
  .isSet = false
};

static struct option long_options[] = {
  {"rgb", required_argument,  0, 'g'},
  {0,     0,                  0, 0}
};

uint8_t str2uint8 (char *str) {
  char *x;
  unsigned long value = 0;
  for (x = str; *x; x++) {
    if (!isdigit(*x)) {
      return 0L;
    }
  }
  value = strtol(str, 0L, 10);
  return (value > 255 ? 255: value);
}

void led_color_rgb(libusb_device_handle* dev_h,
                  uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t buf[] = {
    0x00,0xFF,0x00,0x00,0x00,0x05,0x03,0x01,0x01,
    0x05,   r,   g,   b,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  for (size_t i = 2; i < 88; i++)
    buf[88] ^= buf[i];

  libusb_control_transfer(dev_h, 33, 9, 768, 2, buf, sizeof(buf), 0);
}

void led_color(libusb_device_handle* dev_h, int cmd)
{
  switch (cmd)
  {
  case CMD_COLOR_GREEN:
    led_color_rgb(dev_h, 0, 255, 0);
    break;
  case CMD_COLOR_BLUE:
    led_color_rgb(dev_h, 0, 0, 255);
    break;
  case CMD_COLOR_CYAN:
    led_color_rgb(dev_h, 0, 255, 255);
    break;
  }
}

void led_intensity(libusb_device_handle* dev_h,
                   long intensity)
{

  if (intensity < 0 || intensity > 255)
    return;

  uint8_t v = intensity;

  uint8_t buf[] = {
    0x00,0xFF,0x00,0x00,0x00,0x03,0x03,0x03,0x01,
    0x05,   v,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  for (size_t i = 2; i < 88; i++)
    buf[88] ^= buf[i];

  libusb_control_transfer(dev_h, 33, 9, 768, 2, buf, sizeof(buf), 0);
}

void led_switch(libusb_device_handle* dev_h, int cmd)
{
  if (cmd == CMD_LED_NONE)
    return;

  uint8_t buf[] = {
    0x00,0xFF,0x00,0x00,0x00,0x03,0x03,0x00,0x01,
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  if (cmd == CMD_LED_ON)
    buf[10] = 0x01;

  for (size_t i = 2; i < 88; i++)
    buf[88] ^= buf[i];

  libusb_control_transfer(dev_h, 33, 9, 768, 2, buf, sizeof(buf), 0);
}

void led_breathing_color_pattern(libusb_device_handle* dev_h)
{
  uint8_t buf[] = {
    0x00,0xFF,0x00,0x00,0x00,0x0C,0x03,0x0E,0x01,
    0x05,0x03,0x00,0xFF,0x00,0x00,0xFF,0xFF,0x00,
    0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  for (size_t i = 2; i < 88; i++)
    buf[88] ^= buf[i];

  libusb_control_transfer(dev_h, 33, 9, 768, 2, buf, sizeof(buf), 0);
}

void led_breathing(libusb_device_handle* dev_h, uint8_t cmd)
{
  if (cmd == CMD_BREATHING_NONE)
    return;

  uint8_t buf[] = {
    0x00,0xFF,0x00,0x00,0x00,0x03,0x03,0x02,0x01,
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  switch (cmd)
  {
  case CMD_BREATHING_ON:
    buf[10] = 0x02;
    break;
  case CMD_BREATHING_CYCLING:
    led_breathing_color_pattern(dev_h);
    buf[10] = 0x05;
    break;
  }

  for (size_t i = 2; i < 88; i++)
    buf[88] ^= buf[i];

  libusb_control_transfer(dev_h, 33, 9, 768, 2, buf, sizeof(buf), 0);
}

void operate_device(libusb_device_handle* dev_h)
{
  led_color(dev_h, cmd_color);
  led_switch(dev_h, cmd_led);
  led_breathing(dev_h, cmd_breathing);
  led_intensity(dev_h, cmd_led_intensity);
  if (rgb_values.isSet) {
    led_color_rgb(
            dev_h,
            rgb_values.red,
            rgb_values.green,
            rgb_values.blue);
  }
}

void claim_interface(libusb_device_handle* dev_h)
{
  if (libusb_claim_interface(dev_h, USB_IFACE) != 0)
    return;

  operate_device(dev_h);

  libusb_release_interface(dev_h, USB_IFACE);
}

void open_device(libusb_device* dev)
{
  libusb_device_handle* dev_h;
  if (libusb_open(dev, &dev_h) != 0)
    return;

  if (libusb_set_auto_detach_kernel_driver(dev_h, 1) != LIBUSB_SUCCESS)
    goto exit;

  claim_interface(dev_h);

exit:
  libusb_close(dev_h);
}

void inspect_device(libusb_device* dev)
{
  struct libusb_device_descriptor desc;
  if (libusb_get_device_descriptor(dev, &desc) != 0)
    return;
  if (desc.idVendor == USB_VID && desc.idProduct == USB_PID)
  {
    device_found = 1;
    open_device(dev);
  }
}

int walk_devices()
{
  libusb_device** dev_list;
  ssize_t len = libusb_get_device_list(0, &dev_list);
  if (len < 0)
    return 1;

  for (ssize_t i = 0; i < len; i++)
    inspect_device(dev_list[i]);

  libusb_free_device_list(dev_list, 1);

  if (!device_found)
  {
    fprintf(stderr, "No devices was found\n");
    return 1;
  }

  return 0;
}

int parse_options(int argc, char** argv)
{
  char* help = "Usage: %s [-c green|blue|cyan] [-g <RGBColor>] [-l on|off] [-b on|off|cycling] [-i 0-255]\n"
    " -c Led color: green, blue, cyan\n"
    " -g --rgb Led Color in RGB: 0-255,0-255,0-255\n"
    "    Red Value is ignored, it seems Cynosa Pro doesn't have red leds.\n"
    " -l Led state: on, off\n"
    " -b Led breathing mode: on, off, cycling\n"
    " -i Led intensity: 0-255\n";
  int c;
  char* end;
  int i = 0;
  char *token;
  char *str;
  uint8_t value = 0;

  if (argc < 2)
  {
    fprintf(stderr, help, argv[0]);
    return 1;
  }

  while ((c = getopt_long(argc, argv, "hl:c:b:i:g:", long_options, &option_index)) != -1)
  {
    switch (c)
    {
    case 'h':
      fprintf(stderr, help, argv[0]);
      return 1;
    case 'c':
      if (strcmp(optarg, "green") == 0)
        cmd_color = CMD_COLOR_GREEN;
      else if (strcmp(optarg, "blue") == 0)
        cmd_color = CMD_COLOR_BLUE;
      else if (strcmp(optarg, "cyan") == 0)
        cmd_color = CMD_COLOR_CYAN;
      else
      {
        fprintf(stderr, "Valid -c arguments are: green, blue, cyan\n");
        return 1;
      }
      break;
    case 'g':
      str = strdup(optarg);
      while ((token = strsep(&optarg, ",")))
      {
        value = strcmp(token, "") == 0 ? 0: str2uint8(token);
        switch (i)
        {
          case 0:
            rgb_values.red = value;
            break;
          case 1:
            rgb_values.green = value;
            break;
          case 2:
            rgb_values.blue = value;
            break;
          default:
            break;
        }
        i++;
      }
      free(str);
      rgb_values.isSet = true;
      break;
    case 'l':
      if (strcmp(optarg, "on") == 0)
        cmd_led = CMD_LED_ON;
      else if (strcmp(optarg, "off") == 0)
        cmd_led = CMD_LED_OFF;
      else
      {
        fprintf(stderr, "Valid -l arguments are: on, off\n");
        return 1;
      }
      break;
    case 'b':
      if (strcmp(optarg, "on") == 0)
        cmd_breathing = CMD_BREATHING_ON;
      else if (strcmp(optarg, "off") == 0)
        cmd_breathing = CMD_BREATHING_OFF;
      else if (strcmp(optarg, "cycling") == 0)
        cmd_breathing = CMD_BREATHING_CYCLING;
      else
      {
        fprintf(stderr, "Valid -b arguments are: on, off, cycling\n");
        return 1;
      }
      break;
    case 'i':
      cmd_led_intensity = strtol(optarg, &end, 10);
      if (*end != '\0' || cmd_led_intensity < 0 || cmd_led_intensity > 255)
      {
        fprintf(stderr, "Valid -i arguments are: 0-255\n");
        return 1;
      }
      break;
    case '?':
      if (optopt == 'c' || optopt == 'l' || optopt == 'b'
          || optopt == 'r' || optopt == 'i')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr,
                "Unknown option character `\\x%x'.\n",
                optopt);
      return 1;
    default:
      abort();
    }
  }
  return 0;
}

int main(int argc, char** argv)
{
  if (parse_options(argc, argv) != 0)
    return 1;

  if (libusb_init(NULL) != 0)
  {
    printf("Unable to initialize libusb\n");
    return 1;
  }
  libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_WARNING);
  int ret = walk_devices();
  libusb_exit(NULL);
  return ret;
}
