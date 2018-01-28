/*
 * crypto.h
 *
 *  Created on: Jun 22, 2017
 *      Author: kangidjoel
 */
#include "../secube-host/L1.h"
#ifndef LOCALLIB_HOST_JCRYPTO_H_
#define LOCALLIB_HOST_JCRYPTO_H_

//#define TEST_STRING   ("test_encryption")

int crypto_settime(se3_session* _s);

int get_key_id(char* _key, char** _id, char** _usr);

int set_key(se3_session* _s, uint8_t* _data_key, char* _keyid, char* _usr);

int del_key(se3_session* _s, se3_key* _data_key, char* _key);

int generate_key(size_t _len, uint8_t* buf);

int encrypt_byte(se3_session* _s,
		uint32_t* _sessid,
		uint32_t _keyid,
		uint8_t *_buffer,
		uint16_t _buffer_len,
		uint8_t* _ret,
		uint16_t* _len);
int decrypt_byte(se3_session* _s,
			uint32_t* _sessid,
			uint32_t _keyid,
			uint16_t _buff_len,
			uint16_t enc_buffer_len,
			uint8_t *enc_buffer,
			uint8_t* _ret,
			uint16_t* _len);


#endif /* LOCALLIB_HOST_JCRYPTO_H_ */
