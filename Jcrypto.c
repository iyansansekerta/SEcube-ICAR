/*
 * crypto.c
 *
 *  Created on: Jun 22, 2017
 *      Author: kangidjoel
 */

#include <stdlib.h>
/*user lib*/
#include "../secube-host/L1.h"
#include "Jglobalvar.h"
#include "Jlogger.h"
#include "Jcrypto.h"
#include "Jdbopt.h"

/* Set time for crypto functions */
int crypto_settime(se3_session* _s){

	if (LS3_OK != L1_crypto_set_time(_s, (uint32_t) time(0))) {
		writeLogFile("", "Failure to set time");
		L1_logout(_s);
		return LS3_ER;
	}

	writeLogFile("", "Time set");

	return LS3_OK;
}

int set_key(se3_session* _s, uint8_t* _data_key, char* _keyid, char* _usr){
	uint16_t len=LS3_LENGTH_32;
	se3_key key;
	int keynamelen=strlen(_usr);

	if(strlen(_data_key)==0){

		uint8_t calc[LS3_LENGTH_32];
		memset(calc, 0, LS3_LENGTH_32 * sizeof(uint8_t));

		se3c_rand(LS3_LENGTH_32, calc);

		memcpy(_data_key, calc, strlen(calc));

		memset(calc, 0, sizeof calc);

	}

	strncpy(key.name, _usr, keynamelen);
	key.id = atoi(_keyid);
	key.name_size = keynamelen;
	key.data_size = len;
	key.data = _data_key;
	key.validity = (uint32_t) time(0) + 365 * 24 * 3600; // valid for 1yr

	if (LS3_OK != L1_key_edit(_s, SE3_KEY_OP_UPSERT, &key)) {
		return LS3_ER;
	}

	writeLogFile("", "Key inserted!");

	return LS3_OK;
}

int del_key(se3_session* _s, se3_key* _data_key, char* _key){

	if (LS3_OK != L1_key_edit(_s, SE3_KEY_OP_DELETE, _data_key)) {
			return LS3_ER;
	}

	writeLogFile("", "Key deleted!");

	return LS3_OK;
}

int generate_key(size_t _len, uint8_t* _buf){

	uint8_t calc[LS3_LENGTH_32];
	memset(calc, 0, LS3_LENGTH_32 * sizeof(uint8_t));

	se3c_rand(_len, calc);

	memcpy(_buf,calc,strlen(calc));
	memset(calc, 0, LS3_LENGTH_32 * sizeof(uint8_t));

	return LS3_OK;
}

int encrypt_byte(se3_session* _s,
		uint32_t* _sessid,
		uint32_t _keyid,
		uint8_t *_buffer,
		uint16_t _buffer_len,
		uint8_t* _ret,
		uint16_t* _len){

	uint8_t *enc_buffer = 0;
	uint16_t enc_buffer_len;

	/* Initialize encryption*/
	if (LS3_OK != L1_crypto_init(_s, SE3_ALGO_AES,
			SE3_DIR_ENCRYPT | SE3_FEEDBACK_ECB, _keyid, &_sessid)) {
		writeLogFile("", "Failure initialise crypto session");
		return LS3_ER;
	}

	writeLogFile("", "Encryption Session initialised");

	enc_buffer = (uint8_t*) calloc(ENC_SIZE(_buffer_len), sizeof(uint8_t));

	writeLogFileArr("Original data : \n", "%u, ", _buffer,_buffer_len);

	/*Encrypt and finalise encryption session */
	if (LS3_OK != L1_crypto_update(_s, _sessid,
			SE3_DIR_ENCRYPT | SE3_FEEDBACK_ECB | SE3_CRYPTO_FLAG_FINIT, 0,
			NULL, ENC_SIZE(_buffer_len), _buffer, &enc_buffer_len, enc_buffer)) {
		writeLogFile("", "Failure encrypt data \n");
		return LS3_ER;
	}

	writeLogFile("", "Data encrypted!");
	writeLogFileArr("Encrypted data : \n", "%u, ", enc_buffer,enc_buffer_len);
//	printf("Encrypted data : %s, length %u\n", enc_buffer, enc_buffer_len);


	if (_ret != NULL)
		strncpy(_ret, enc_buffer, enc_buffer_len);

	if (_len != NULL)
		*_len = enc_buffer_len;


	return LS3_OK;
}

int decrypt_byte(se3_session* _s,
			uint32_t* _sessid,
			uint32_t _keyid,
			uint16_t _buff_len,
			uint16_t enc_buffer_len,
			uint8_t *enc_buffer,
			uint8_t* _ret,
			uint16_t* _len){


	uint8_t *dec_buffer;
	uint16_t dec_buff_len = 0;

	if (LS3_OK != L1_crypto_init(_s, SE3_ALGO_AES,
			SE3_DIR_DECRYPT | SE3_FEEDBACK_ECB, _keyid, &_sessid)) {
		writeLogFile("", "Failure initialise crypto session");
		return LS3_ER;
	} else {
		writeLogFile("", "Decryption Session initialised");
	}

	dec_buffer = (uint8_t*) calloc(ENC_SIZE(_buff_len), sizeof(uint8_t));

	/* Decrypt and finalise encryption session */
	if (LS3_OK != L1_crypto_update(_s, _sessid,
			SE3_DIR_DECRYPT | SE3_FEEDBACK_ECB | SE3_CRYPTO_FLAG_FINIT, 0,
			NULL, enc_buffer_len, enc_buffer, &dec_buff_len, dec_buffer)) {
		writeLogFile("", "Failure to decrypt data");
		return LS3_ER;
	} else {
		//printf("Data %s, length %d\n", dec_buffer, dec_buff_len);
		writeLogFile("", "Data decrypted");
	}

	if(_ret!=NULL)
		strncpy(_ret,dec_buffer,dec_buff_len);

	if(_len!=NULL)
		*_len = dec_buff_len;

	return LS3_OK;
}
