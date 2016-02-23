#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <firestorm.h>
#include <tock.h>

const driver_t CONSOLE = { num: 0 };
const driver_t GPIO = { num: 1};
const driver_t TIMER = { num: 2};
const driver_t TEMPERATURE = { num: 3};
const driver_t ACCELEROMETER = { num: 4};
const driver_t SPI = { num: 5};

int gpio_enable(unsigned int pin) {
  return command(GPIO, 0, pin);
}

int gpio_set(unsigned int pin) {
  return command(GPIO, 2, pin);
}

int gpio_clear(unsigned int pin) {
  return command(GPIO, 3, pin);
}

int gpio_toggle(unsigned int pin) {
  return command(GPIO, 4, pin);
}

static CB_TYPE putstr_cb(int _x, int _y, int _z, void* str) {
  free(str);
  return PUTSTR;
}

void putnstr(const char *str, size_t len) {
  char* buf = (char*)malloc(len * sizeof(char));
  strncpy(buf, str, len);
  putnstr_async(buf, len, putstr_cb, buf);
  wait_for(PUTSTR);
}

void putnstr_async(const char *str, size_t len, subscribe_cb cb, void* userdata) {
  allow(CONSOLE, 1, (void*)str, len);
  subscribe(CONSOLE, 1, cb, userdata);
}

void putstr(const char *str) {
  putnstr(str, strlen(str));
}

int timer_oneshot_subscribe(subscribe_cb cb, void *userdata) {
  return subscribe(TIMER, 0, cb, userdata);
}

int timer_repeating_subscribe(subscribe_cb cb, void *userdata) {
  return subscribe(TIMER, 1, cb, userdata);
}

int spi_write_byte(unsigned char byte) {
  return command(SPI, 0, byte);
}

int spi_read_buf(const char* str, size_t len) {
  allow(SPI, 0, (void*)str, len);
}

static CB_TYPE spi_cb(int r0, int r1, int r2, void* ud) {
  return SPIBUF;
}

int spi_write(const char* str, 
   	      size_t len, 
	      subscribe_cb cb) { 
  allow(SPI, 1, (void*)str, len);
  subscribe(SPI, 0, cb, NULL);
  command(SPI, 1, len);
}

int spi_read_write(const char* write,
		   char* read, 
		   size_t  len,
		   subscribe_cb cb) {

  allow(SPI, 0, (void*)read, len);
  spi_write(write, len, cb);
}

/** FXOS8700CQ: magnetometer, accelerometer **/

int FXOS8700CQ_accel_enable() {
  return command(ACCELEROMETER, 1, 0);
}

int FXOS8700CQ_magnet_enable() {
  return command(ACCELEROMETER, 2, 0);
}

static CB_TYPE FXOS8700CQ_read_accel_cb(int r0, int r1, int r2, void* ud) {
  accel_result_t *res = (accel_result_t*)ud;
  res->x = r0;
  res->y = r1;
  res->z = r2;
  return READACCEL;
}

int FXOS8700CQ_accel_read(accel_result_t *res) {
  int error = FXOS8700CQ_accel_read_async(FXOS8700CQ_read_accel_cb, (void*)res);
  if (error < 0) {
    return error;
  }
  wait_for(READACCEL);
  return 0;
}

int FXOS8700CQ_accel_read_async(subscribe_cb cb, void* userdata) {
    return subscribe(ACCELEROMETER, 1, cb, userdata);
}

static CB_TYPE FXOS8700CQ_read_magnet_cb(int r0, int r1, int r2, void* ud) {
  magnet_result_t *res = (magnet_result_t*)ud;
  res->x = r0;
  res->y = r1;
  res->z = r2;
  return READMAGNET;
}

int FXOS8700CQ_magnet_read(magnet_result_t *res) {
  int error = FXOS8700CQ_magnet_read_async(FXOS8700CQ_read_magnet_cb, (void*)res);
  if (error < 0) {
    return error;
  }
  wait_for(READMAGNET);
  return 0;
}

int FXOS8700CQ_magnet_read_async(subscribe_cb cb, void* userdata) {
    return subscribe(ACCELEROMETER, 2, cb, userdata);
}

int spi_block_write(char* str, 
		    size_t len) { 
    spi_write(str, len, spi_cb);
}
