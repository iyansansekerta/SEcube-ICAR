/*
 * user.c
 *
 *  Created on: Jun 12, 2017
 *      Author: kangidjoel
 *  1. login (@param 1, @param pin)
 *  2. change pin (@param 2, @param old pin, @param pin)
 *  2.1. req : old pin
 *  3. checksum with sha256 or list file from db (+restore)(@param 3, @param option(1:get_file_list from db;2:get_hash_file from db), @param file1, @param file2)
 *  4. copy file to secube storage (hidden) (@param 4, @param username, @param pin, @param path\host_filename)
 *  4.1. req : pin
 *  4.2. hidden file after copy
 *  4.3. verify copy file using sha256
 *  5. SeCube crypto function (@param 5, @param user, @param pin, @param(1:encrypt;2:decrypt), @param keyid, @param string)
 *  6. Key management function (@param 6, @param user, @param pin, @param(1:set_key;2:get_key;3:del_key;4:gen_key), @param keyid, @param string)
 */

#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
/* user lib */
#include "../secube-host/L0.h"
#include "../secube-host/L1.h"
#include "Jglobalvar.h"
#include "Jsefunction.h"
#include "Jcrypto.h"

static uint8_t pin_default[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static uint8_t pin_user[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static uint8_t pin[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

char *passwd;
int keyid;
se3_disco_it it;
static se3_device_info device_info;
se3_device dev;
se3_session s;
//uint16_t return_value = 0;

bool logged_in;
int ret_code;
int bypass_log; // bypass login

int login(uint16_t _access, int _bypass);
int change_pin(uint16_t _access, uint8_t* pin);

int call_process(int arr_size, char * args[]) {

	/*L0_discover_init(&it);
	 if (L0_discover_next(&it)) {*/
	logged_in = FALSE;
	ret_code = LS3_OK;
	bypass_log = LS3_ONE; // bypass state 1:TRUE;0:FALSE
	char msg[LS3_LENGTH_500];
	int param[] = { 00 };

	memset(msg, 0, LS3_LENGTH_500 * sizeof(char));

	param[0] = (int) strtol(args[1], NULL, 10);
	//printf("param 0 : %d\n", param[0]);

	/*1. login (@param 1, @param access(1:ADMIN;2:USER), @param keyid, @param user_role_pin, @encrypted_pin)*/
	if (param[0] == 1) {
		writeLogFile("", "Requesting login...\n");

		param[1] = (int) strtol(args[2], NULL, 10);
		/*printf("param 1 : %d\n",param[1]);*/

		passwd = args[4];
		arrtoarr(passwd, pin);

		/*pin : test*/
		if (login(SE3_ACCESS_USER, bypass_log) != LS3_OK)
			return LS3_ER;

		logged_in = true;

		ret_code = crypto_settime(&s);

		if (ret_code != LS3_OK)
			goto cleanup;

		int str_loc = 5;
		keyid = (int) strtol(args[3], NULL, 10); // keyid

		uint8_t *buffer;
		uint16_t buffer_len = 0;
		uint32_t session_id = 0;

		uint8_t* ret = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));
		uint16_t len = 0;

		buffer_len = arr_size - str_loc;
		buffer = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));

		arrtouint(str_loc, buffer_len, args, buffer); //copy array start from array-i
		writeLogFileArr("Buffer : ", "%u ", buffer, SE3_L1_CRYPTOBLOCK_SIZE);
		ret_code = decrypt_byte(&s, session_id, keyid, 8, buffer_len, buffer,
				ret, &len);
//		writeLogFile("", concat("String return ", ret));

		if (logged_in)
			logout();

		logged_in = false;

		strncpy(pin, ret, strlen(ret));

		/*pin : "LS3_2018"*/
		if (login(SE3_ACCESS_ADMIN, bypass_log) != LS3_OK)
			return LS3_ER;

		logged_in = true;
	}
	/*2. change pin (@param 2, @param access(1:ADMIN;2:USER), @param admin pin, @param admin/user pin)*/
	else if (param[0] == 2) {

		writeLogFile("", "Requesting change pin... 2\n");
		param[1] = (int) strtol(args[2], NULL, 10);

		passwd = args[3];
		arrtoarr(passwd, pin);

		/*pin : "LS3_2018"*/
		if (login(SE3_ACCESS_ADMIN, bypass_log) != LS3_OK)
			return LS3_ER;

		logged_in = true;
		strncpy(pin, pin_default, 32);

		passwd = args[4];
		for (int i = 0; i < strlen(passwd); i++) {
			pin[i] = passwd[i];
		}

		if (param[1] == 1) {
			if (LS3_OK != change_pin(SE3_ACCESS_ADMIN, pin))
				return LS3_ER;
		} else if (param[1] == 2) {
			if (LS3_OK != change_pin(SE3_ACCESS_USER, pin))
				return LS3_ER;
		}

		writeLogFile("", "PIN Change");

	}
	/*3. checksum with sha256/list file from db (+restore) (@param 3, @param option(1:get_file_list from db;2:integrity_check), @param path\host_filename), @param restore, @recursive*/
	else if (param[0] == 3) {
		writeLogFile("", "Requesting integrity check...\n");

		char *device_path = malloc(LS3_LENGTH_256);
		param[1] = (int) strtol(args[3], NULL, 10); // restore deleted[1]
		param[2] = (int) strtol(args[4], NULL, 10); // recursive scan[1]

		writeLogFile("", "Checking Database ");

		int open = open_device(bypass_log);

		if (open==LS3_OK)
			sprintf(device_path, "%ls\\%s\\", device_info.path, LS3_PATH);
		else
			sprintf(device_path, "%s\\", LS3_PATH);

		create_rec_dir(args[2]);

		integrity_test_2(args[2], device_path, param[1], param[2]);
//		integrity_test(args[2], device_path);

		if(open==LS3_OK){
			L0_close(&dev);
		}

		return LS3_OK;
	}
	/*4. copy file to secube storage (hidden) (@param 4, @param username, @param pin, @param source_path\host_filename @param dest_path, @param id, @param encrypted_pin)*/
	else if (param[0] == 4) {
		writeLogFile("", "Requesting backup file...\n");

		passwd = args[3];
		arrtoarr(passwd, pin);

		/*pin : test */
		if (login(SE3_ACCESS_USER, bypass_log) != LS3_OK) {
			return LS3_ER;
		}

		logged_in = true;

		int str_loc = 7; // array[str_loc] of encrypted strings

		keyid = (int) strtol(args[6], NULL, 10); // keyid

		uint8_t *buffer;
		uint16_t buffer_len = 0;
		uint32_t session_id = 0;

		uint8_t* ret = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));
		uint16_t len = 0;

		buffer_len = arr_size - str_loc;
		buffer = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));

		arrtouint(str_loc, buffer_len, args, buffer); //copy array start from array-i
//		writeLogFileArr("Buffer : ", "%u ", buffer, SE3_L1_CRYPTOBLOCK_SIZE);

		if (bypass_log != 1) {
			ret_code = crypto_settime(&s);

			if (ret_code != LS3_OK)
				goto cleanup;

			ret_code = decrypt_byte(&s, session_id, keyid, 8, buffer_len,
					buffer, ret, &len);
//			writeLogFile("", concat("String return ", ret));

			if (logged_in)
				logout();
		}

		logged_in = false;

		strncpy(pin, ret, strlen(ret));

		/*pin : adm123*/
		if (login(SE3_ACCESS_ADMIN, bypass_log) != LS3_OK)
			return LS3_ER;

		logged_in = true;

		// backup process start
		int stringlen =
				(strlen(device_info.path) != 0) ?
						LS3_LENGTH_DEV_PATH_1 : LS3_LENGTH_DEV_PATH_0;
		char device_path[stringlen];
		memset(device_path, 0, stringlen * sizeof(char));

		if (strlen(device_info.path) != 0)
			snprintf(device_path, stringlen, "%ls\\%s", device_info.path,
					LS3_PATH);
		else
			snprintf(device_path, stringlen, "%s", LS3_PATH);

		device_path[stringlen] = 0;

		/*one file test
		 backup_file(args[2],args[4],args[5],device_path);*/

		backup_file_rec(args[2], args[4], args[5], device_path);

		if (logged_in && bypass_log != 1)
			logout();

		logged_in = false;

		memset(msg, 0, sizeof msg);

	}
	/*5. SeCube crypto function (@param 5, @param user, @param pin, @param(1:decrypt;2:encrypt), @param keyid, @param string)*/
	else if (param[0] == 5) {
		writeLogFile("", "Requesting cryptography function...\n");

		passwd = args[3];
		param[1] = (int) strtol(args[4], NULL, 10); // decrypt/encrypt
		keyid = (int) strtol(args[5], NULL, 10); // keyid
		int str_loc = 6; // start position of array contain (decrypt/encrypt) string
		arrtoarr(passwd, pin); // override pin

		/*pin : test*/
		if (login(SE3_ACCESS_USER, bypass_log) != LS3_OK)
			return LS3_ER;

		logged_in = true;

		uint32_t session_id = 0;
		uint8_t *buffer;
		uint16_t buffer_len = 0;

		bool opt;
		uint8_t* ret = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));
		uint16_t len = 0;

		opt = (param[1] == 1 && logged_in) ? true : false;
		buffer_len = (opt) ? arr_size - str_loc /*char array start from array-i*/: strlen(args[str_loc]);
		buffer = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));

		ret_code = crypto_settime(&s);

		if (ret_code != LS3_OK)
			goto cleanup;

		if (opt) {
			arrtouint(str_loc, buffer_len, args, buffer); //copy array start from array-i
			writeLogFileArr("Buffer : ", "%u ", buffer,	SE3_L1_CRYPTOBLOCK_SIZE);
			ret_code = decrypt_byte(&s, session_id, keyid, 8, buffer_len, buffer, ret, &len);
		} else /*if (param[1] == 2 && logged_in)*/{
			strncpy(buffer, args[str_loc], buffer_len);
			ret_code = encrypt_byte(&s, session_id, keyid, buffer, buffer_len, ret, &len);
		}

		if (ret_code != LS3_OK) {
			goto cleanup;
		}

		writeLogFile("", concat("String return ", ret));
		writeLogFileArr("String key : \n", "%u, ", ret, len);
	}
	/*6. Key management function (@param 6, @param user, @param pin, @param(1:set_key;2:get_key;3:del_key;4:gen_key), @param keyid, @param string)*/
	else if (param[0] == 6) {
		writeLogFile("", "Requesting key management function...\n");
		passwd = args[3];
		param[1] = (int) strtol(args[4], NULL, 10); // set/get/del/gen
		keyid = (int) strtol(args[5], NULL, 10); // keyid
		arrtoarr(passwd, pin); // override pin

		/*pin : LS3_TEST*/
		if (login(SE3_ACCESS_USER, bypass_log) != LS3_OK)
			return LS3_ER;

		logged_in = true;

		/* set key */
		if (param[1] == 1 && logged_in) {
			uint8_t data_key[LS3_LENGTH_32];

			memset(data_key, 0, LS3_LENGTH_32 * sizeof(uint8_t));
			memcpy(data_key, args[6], strlen( args[6]));

			ret_code = set_key(&s, data_key, args[5], args[2]);

			if (ret_code != LS3_OK) {
				sprintf(msg, "Failed to insert key!");
			}

			memset(data_key, 0, sizeof data_key);
		}
		/* check key id&user*/
		else if (param[1] == 2 && logged_in) {

			ret_code = (LS3_find_key_user(&s, keyid, args[2])) ? 0 : 1;
			if (ret_code != LS3_OK) {
				sprintf(msg, "Key not found!");
			}else {
				sprintf(msg, "Key found!");
				ret_code = LS3_ONE; // so the system write to log :D
			}

//			writeLogFile("", msg);

		}
		/* delete key */
		else if (param[1] == 3 && logged_in) {
			se3_key key;
			int keynmlen = strlen(args[2]);
			strncpy(key.name, args[2], keynmlen);
			key.id = keyid;
			key.name_size = strlen(key.name);
			key.data_size = LS3_LENGTH_32;
			key.data = args[6];
			ret_code = del_key(&s, &key, args[5]);

			if (ret_code != LS3_OK) {
				sprintf(msg, "Key cannot/already deleted!");
			}
		}
		/* generate key */
		else if (param[1] == 4 && logged_in) {
			uint8_t key[LS3_LENGTH_32];
			memset(key, 0, LS3_LENGTH_32 * sizeof(uint8_t));
			generate_key(LS3_LENGTH_32, key);
			writeLogFile("",concat("Generated key : " ,key));
			memset(key, 0, LS3_LENGTH_32 * sizeof(uint8_t));
		}

	}
	/*7. remove file (@param 7, @param username, @param pin, @param source_path\host_filename, @param id, @param encrypted_pin)*/
	else if (param[0] == 7) {

		writeLogFile("", "Requesting remove file...\n");

		passwd = args[3];
		arrtoarr(passwd, pin);

		int stringlen = LS3_LENGTH_1000;
		char sql[stringlen];
		char fname[LS3_LENGTH_DEV_PATH_1];

		memset(sql, 0, stringlen * sizeof(char));
		memset(fname, 0, LS3_LENGTH_DEV_PATH_1 * sizeof(char));

		memcpy(sql, LS3_QGET_FNME, strlen(LS3_QGET_FNME));
		sql[stringlen] = 0;

		replace(sql, "&fnme", args[4]);

		if (get_db_data(sql, fname, NULL, NULL) != LS3_OK){
			sprintf(msg,"%s", "File not found!");
			ret_code = LS3_ER;
		}

		memset(sql, 0, sizeof sql);

		if (ret_code!=LS3_OK){
			goto cleanup;
		}

		/*pin : test */
		if (login(SE3_ACCESS_USER, bypass_log) != LS3_OK) {
			return LS3_ER;
		}

		logged_in = true;

		int str_loc = 6; // array[str_loc] of encrypted strings

		keyid = (int) strtol(args[5], NULL, 10); // keyid

		uint8_t *buffer;
		uint16_t buffer_len = 0;
		uint32_t session_id = 0;

		uint8_t* ret = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));
		uint16_t len = 0;

		buffer_len = arr_size - str_loc;
		buffer = (uint8_t*) calloc(ENC_SIZE(buffer_len), sizeof(uint8_t));

		arrtouint(str_loc, buffer_len, args, buffer); //copy array start from array-i

		if (bypass_log != 1) {
			ret_code = crypto_settime(&s);

			if (ret_code != LS3_OK)
				goto cleanup;

			ret_code = decrypt_byte(&s, session_id, keyid, 8, buffer_len,
					buffer, ret, &len);
			//			writeLogFile("", concat("String return ", ret));

			if (logged_in)
				logout();
		}

		logged_in = false;

		strncpy(pin, ret, strlen(ret));

		/*pin : LS3_2018*/
		if (login(SE3_ACCESS_ADMIN, bypass_log) != LS3_OK)
			return LS3_ER;

		logged_in = true;

		if (logged_in) {
			if(remove_file(args[4],device_info.path)!=LS3_OK){
				ret_code = LS3_ER;
				sprintf(msg, "%s", "Remove failed!");
			}
		}

	}else if (param[0] == 8) {

		passwd = args[2];
		arrtoarr(passwd, pin);

		/*pin : LS3_2018 */
		if (login(SE3_ACCESS_ADMIN, bypass_log) != LS3_OK) {
			return LS3_ER;
		}

		logged_in = true;

		if (logged_in) {
			DIR *d;
			struct dirent *dir;
			char source[LS3_LENGTH_256];
			int len;

			memset(source, 0, LS3_LENGTH_256 * sizeof(char));
			sprintf(source, "%ls\\", device_info.path);

			d = opendir(source);

			if (d) {
				while ((dir = readdir(d)) != NULL) {

					len = strlen(dir->d_name);

					if (strncmp(dir->d_name, ".", len) != 0
							&& strncmp(dir->d_name, "..", len) != 0) {

						printf("%s\n", dir->d_name);

						writeLogFile("", concat("File source : ", dir->d_name));


					}

				}

				closedir(d);
			}

			memset(source, 0, LS3_LENGTH_256 * sizeof(char));
		}

	}
	/*} else {
	 writeLogFile("", "Device not found!");
	 return LS3_ER;
	 }*/

	cleanup:

	if (ret_code != LS3_OK) {
		writeLogFile("", msg);
	}

	if (logged_in)
		logout();

	return ret_code;
}

// Open device
int open_device(int _bypass) {

	writeLogFile("", "Looking for SEcube devices...");

	if (_bypass == 1) {
		writeLogFile("", "Login BYPASS..\n");
	} else if (_bypass == 0) {
		L0_discover_init(&it);
		if (L0_discover_next(&it)) {
			writeLogFile("", "SEcube device found!");
			device_info = it.device_info;
			if (L0_open(&dev, &(it.device_info), SE3_TIMEOUT) != LS3_OK) {
				writeLogFile("", "Error opening device.\nPlease check board connection.\n");
				return LS3_ER;
			} else {
				writeLogFile("", "Open Device success");
			}
		} else {
			writeLogFile("", "Error, device not found!.\n");
			return LS3_ER;
		}
	}

	return (LS3_OK);
}

// Log in
int login(uint16_t _access, int _bypass) {

	if (LS3_OK != open_device(_bypass))
		return LS3_ER;
	else if (_bypass == 1) {
	} else {

		writeLogFile("",
				concat("Permission to ",
						(_access == SE3_ACCESS_ADMIN) ? "WRITE" : "ACCESS"));

		if (L1_login(&s, &dev, pin, _access) != LS3_OK) {
			writeLogFile("",
					"Error, login failed.\nPlease check security pin.");
			return LS3_ER;
		} else {
			writeLogFile("", "Login success!");
			logged_in = true;
		}

		if (logged_in)
			writeLogFile("", "You are logged in SEcube device!");
	}

	return (LS3_OK);
}

// Log out
int logout() {

	writeLogFile("", "Logging out…");
//	Sleep(3000);
	ret_code = L1_logout(&s);
	if (ret_code != LS3_OK) {
		writeLogFile("", "Error, logout failed.\nPlease check board connection.\n");
		ret_code = LS3_ER;
	} else {
		writeLogFile("", "Logout success");
		writeLogFile("", "\n\nConnection to SEcube device was successful");
		ret_code = LS3_OK;
	}

	L0_close(&dev);

	return ret_code;

}

int change_pin(uint16_t _access, uint8_t* _pin) {
	if (_access == SE3_ACCESS_ADMIN) {
		if (LS3_OK != L1_set_admin_PIN(&s, _pin)) {
			return LS3_ER;
		}
	} else if (_access == SE3_ACCESS_USER) {
		if (LS3_OK != L1_set_user_PIN(&s, _pin)) {
			return LS3_ER;
		}
	} else
		return LS3_ER;

	writeLogFile("", "Change PIN Success!");

	return LS3_OK;
}
