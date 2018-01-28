/*
 * user.h
 *
 *  Created on: Jun 12, 2017
 *      Author: kangidjoel
 */

#include <stdint.h>



int call_process(int arr_size,char * args[]);
// Open device
//int open_device(int _bypass);
// Log in
int login(uint16_t _access, int _bypass);
// Log out
int logout();
// change pin
int change_pin(uint16_t _access, uint8_t* pin);
