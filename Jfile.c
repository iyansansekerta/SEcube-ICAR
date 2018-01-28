/*
 * FileCopy.c
 *
 *  Created on: Jun 10, 2017
 *      Author: kangidjoel
 */

#include<stdio.h>
#include<stdlib.h>
#include<process.h>
#include <dirent.h>

#include <time.h>
#include <sys/utime.h>
#include <sys/stat.h>
#include <unistd.h>

/* user lib */
#include "Jlogger.h"
#include "Jglobalvar.h"
#include "Jdbopt.h"
#include "Jfile.h"
#include "../secube-host/sha256.h"

int compare_file(char* _filename1, char * _filename2) {

	char results1[LS3_LENGTH_64];
	char results2[LS3_LENGTH_64];

	memset(results1, 0, LS3_LENGTH_64 * sizeof(char));
	memset(results2, 0, LS3_LENGTH_64 * sizeof(char));

	get_file_hash(_filename1, results1);

	get_file_hash(_filename2, results2);

	if (strncmp(results2, results1, LS3_LENGTH_64) != LS3_OK)
		return LS3_ER;

	return LS3_OK;
}

int compare_hash(char* _filename, char *_hash) {

	char results[LS3_LENGTH_64];
	memset(results, 0, LS3_LENGTH_64 * sizeof(char));

	if (get_file_hash(_filename, results) != LS3_OK) {
		writeLogFile("", "Get file hash encounter problem!");
		return LS3_ER;
	}

	if (strncmp(results, _hash, B5_SHA256_BLOCK_SIZE) != LS3_OK)
		return LS3_ER;

	return LS3_OK;
}

int remove_file(char* _fname, char* _device_info){

	int stringlen = LS3_LENGTH_1000;
	char sql[stringlen];
	memset(sql, 0, stringlen * sizeof(char));
	memcpy(sql, LS3_QDEL_FILE, strlen(LS3_QDEL_FILE));
	sql[stringlen] = 0;

	replace(sql, "&path", _fname);
//	writeLogFile("", sql);

	int LS3_RET = set_db_data(sql);

	memset(sql, 0, sizeof sql);

	if (LS3_RET==LS3_OK){

		char falias[LS3_LENGTH_128];
		char hcde[LS3_LENGTH_64];

		uint16_t lenc = LS3_ZERO;

		memset(falias, 0, LS3_LENGTH_128 * sizeof(char));
		memset(hcde, 0, LS3_LENGTH_64 * sizeof(char));

		crypto_filename(_fname, falias, &lenc);

		if (get_file_hash(_fname, hcde) != LS3_OK) {
			writeLogFile("", "Get file hash encounter problem!");
			return LS3_ER;
		}

		snprintf(falias, LS3_LENGTH_128, "%s%s", basename(falias), hcde);

		stringlen =	(strlen(_device_info) != 0) ? LS3_LENGTH_148 : LS3_LENGTH_DEV_PATH_0;
		char device_path[stringlen];
		memset(device_path, 0, stringlen * sizeof(char));

		if (strlen(_device_info) != 0){
			snprintf(device_path, LS3_LENGTH_148, "%ls\\%s\\%s", _device_info, LS3_PATH, falias);
			device_path[LS3_LENGTH_148] = 0;
		}else{
			sprintf(device_path, "%s\\%s", LS3_PATH, falias);
			device_path[LS3_LENGTH_DEV_PATH_0] = 0;
		}

//		printf("%s",device_path);

		set_file_readonly(device_path, 0);

		remove(device_path);

		memset(falias, 0, sizeof falias);
		memset(hcde, 0, sizeof hcde);
		memset(device_path, 0, sizeof device_path);
	}

	return LS3_RET;
}

int backup_file(char* _user, char* _source_path, char* _dest_path,
		char* _device_info) {

	uint16_t lenc = LS3_ZERO;

	char msg[LS3_LENGTH_500];

	memset(msg, 0, LS3_LENGTH_500 * sizeof(char));

	int stringlen = strlen(_source_path); // source path
	int trials = LS3_THREE;
	char hcde[LS3_LENGTH_64];
	char falias[LS3_LENGTH_128];
	char temp[LS3_LENGTH_100];

	memset(falias, 0, LS3_LENGTH_128 * sizeof(char));
	memset(temp, 0, LS3_LENGTH_100 * sizeof(char));
	memset(hcde, 0, LS3_LENGTH_64 * sizeof(char));

	stringlen = strlen(basename(_source_path));
	char fname[stringlen];
	memset(fname, 0, stringlen * sizeof(char));
	memcpy(fname, basename(_source_path), stringlen);
	fname[stringlen] = 0;

	stringlen = strlen(_dest_path); // destination path
	char dest_path[stringlen];
	memset(dest_path, 0, stringlen * sizeof(char));
	memcpy(dest_path, _dest_path, stringlen);
	dest_path[stringlen] = 0;

	if (get_file_hash(_source_path, hcde) != LS3_OK) {
		writeLogFile("", "Get file hash encounter problem!");
		return LS3_ER;
	}

	stringlen = LS3_LENGTH_1000;
	char sql[stringlen];
	memset(sql, 0, stringlen * sizeof(char));
	memcpy(sql, LS3_QGET_FNME, strlen(LS3_QGET_FNME));
	sql[stringlen] = 0;

	replace(sql, "&fnme", dest_path);
//	writeLogFile("", sql);

	get_db_data(sql, falias, NULL, NULL);

	if (falias[0] == 0) {
		crypto_filename(fname, falias, &lenc);
		snprintf(falias, LS3_LENGTH_128, "%s%s", falias, hcde);
	}

	stringlen = (strlen(_device_info) != strlen(LS3_PATH)) ? LS3_LENGTH_DEV_PATH_1 : LS3_LENGTH_DEV_PATH_0;
	char device_path[stringlen + 1];
	memset(device_path, 0, stringlen * sizeof(char));

	snprintf(device_path, stringlen, "%s\\%s", _device_info, falias);

	device_path[stringlen] = 0;

	set_file_readonly(device_path, 0);

	writeLogFile("", "Copying file... please wait...");

	copy_process:

	if (copy_file2(_source_path, device_path) != LS3_OK) {
		sprintf(msg, "Cannot copy file!\nsource path: %s\ndevice path: %s",
				_source_path, device_path);
		writeLogFile("", msg);
		writeLogFile("", "Try to fix attribute file...");

//		return LS3_ER;
		if (trials > 0) {
			trials--;
			goto copy_process;
		}

		return LS3_ER;
	}

	trials = LS3_THREE; // reset trials

	writeLogFile("", "Verifying backup file...");
	if (compare_file(_source_path, device_path) != LS3_OK) {
		writeLogFile("", "Hash file not match!, repeating copy...");
		if (trials > 0) {
			trials--;
			goto copy_process;
		}

		writeLogFile("","Reaching maximum trial, copy file failed!\nBackup sequence terminated!");

		return LS3_ER;
	}

	trials = LS3_THREE; // reset trials
	attribreset:
	/* set attribute file to readonly */
	if (set_file_readonly(device_path, 1) != LS3_OK) {
		if (trials > 0) {
			trials--;
			goto attribreset;
		}
		sprintf(msg, "Set attribute fail!\ndevice path: %s", device_path);
		writeLogFile("", msg);
	}

	writeLogFile("", "Verified OK!\nSynchronizing database...");

	memset(sql, 0, sizeof sql);
	memcpy(sql, LS3_QSET_HCDE, strlen(LS3_QSET_HCDE));

	replace(sql, "&hcde", hcde);
	replace(sql, "&fnme", basename(_source_path));
	replace(sql, "&user", _user);
	replace(sql, "&fals", basename(device_path));
	replace(sql, "&lupd", get_modified_file(_source_path, 0));
	sprintf(temp, "%d", get_file_size(_source_path));
	replace(sql, "&fsiz", temp);
	replace(sql, "&dsph", dirname(dest_path));

//	writeLogFile("", sql);

	set_db_data(sql);

	writeLogFile("", "Backup sequence complete!");

	memset(device_path, 0, sizeof device_path);
	memset(dest_path, 0, sizeof dest_path);
	memset(sql, 0, sizeof sql);
	memset(hcde, 0, sizeof hcde);
	memset(fname, 0, sizeof fname);
	memset(falias, 0, sizeof falias);

	return LS3_OK;

}

int backup_file_rec(char* _user, char* _source_path, char* _dest_path,
		char* _device_info) {

	DIR *d, *sd;
	struct dirent *dir;
	char source[LS3_LENGTH_256], dest[LS3_LENGTH_256];
	int len;

	d = opendir(_source_path);

	if (d) {
		while ((dir = readdir(d)) != NULL) {

			len = strlen(dir->d_name);

			if (strncmp(dir->d_name, ".", len) != 0
					&& strncmp(dir->d_name, "..", len) != 0) {

				memset(source, 0, LS3_LENGTH_256 * sizeof(char));
				memset(dest, 0, LS3_LENGTH_256 * sizeof(char));

				sprintf(source, "%s\\%s", _source_path, dir->d_name);
				sprintf(dest, "%s\\%s", _dest_path, dir->d_name);

				sd = opendir(source);
				if (sd) {

					backup_file_rec(_user, source, dest, _device_info);

					closedir(sd);

				} else {
					backup_file(_user, source, dest, _device_info);

					writeLogFile("", concat("File source : ", source));
				}

				memset(source, 0, LS3_LENGTH_256 * sizeof(char));
				memset(dest, 0, LS3_LENGTH_256 * sizeof(char));
			}

		}

		closedir(d);
	}

	return LS3_OK;
}

int integrity_test(char* _filename, char* _socpath, int _restore) {

	int fnamelen = strlen(_filename);

	char dbhash[LS3_LENGTH_64];
	char pLupd[LS3_LENGTH_19];
	char pHash[LS3_LENGTH_64];
	char dbfsize[LS3_LENGTH_19];
	char filename[fnamelen];
	char lLupd[LS3_LENGTH_14];

	int fsize;
	int dbsize;

	memset(dbhash, 0, LS3_LENGTH_64 * sizeof(char));
	memset(pLupd, 0, LS3_LENGTH_19 * sizeof(char));
	memset(pHash, 0, LS3_LENGTH_64 * sizeof(char));
	memset(filename, 0, fnamelen * sizeof(char));
	memset(dbfsize, 0, LS3_LENGTH_19 * sizeof(char));
	memset(lLupd, 0, LS3_LENGTH_14 * sizeof(char));

	memcpy(filename, _filename, fnamelen);
	filename[fnamelen] = 0;

	if (get_file_hash(_filename, pHash) != LS3_OK) {
		return LS3_ER;
	}

	if (_restore==0){
		/* jika hash file tidak terdaftar di db */
		if (get_db_hash_by_hfile(pHash, dbhash, pLupd, dbfsize) != LS3_OK) {
			writeLogFile("", concat(filename, " not listed in database!, removing file..."));
			set_file_readonly(filename, 0);
			remove(filename);
			return LS3_OK;
		}
	} else {
		/* jika file tidak terdaftar di db */
		if (get_db_hash(filename, dbhash, pLupd, dbfsize) != LS3_OK) {
			writeLogFile("", concat(filename, " not listed in database!, removing file..."));
			set_file_readonly(filename, 0);
			remove(filename);
			return LS3_OK;
		}
	}

	if (!dbhash && !pLupd && !dbfsize) {
		return LS3_ER;
	}

	memcpy(lLupd, pLupd, strlen(pLupd));
	lLupd[strlen(pLupd)] = 0;

	dbsize = (long) strtol(dbfsize, NULL, 10);

	char *modify = (_restore == 1) ? get_modified_file(_filename, 1):lLupd;
	fsize = (_restore == 1) ? get_file_size(_filename):dbsize;

	if (strncmp(pHash, dbhash, B5_SHA256_BLOCK_SIZE) != LS3_OK
			|| strncmp(modify, lLupd, strlen(lLupd)) != LS3_OK
			|| fsize != dbsize) {

		/* if hash results are not the same, initiate restore sequence :
		 * 1. compare hash backup file with db
		 * 2. if match, restore backup file to destination
		 * 3. verify hash restore file with db hash
		 * 4. if not match, repeat restore sequence, else finish the sequence
		 * */

		writeLogFile("", concat(_filename, " has been removed or modified!\n\nInitiate restore sequence..."));

		/* remove modified/illegal file */
		remove(_filename);

		int trial = 3;

		pHash[0] = 0;
		char fname[LS3_LENGTH_DEV_PATH_1];
		memset(fname, 0, LS3_LENGTH_DEV_PATH_1 * sizeof(char));

		repeat_copy:

		writeLogFile("", "Verifying backup file...");

		/* get filename */
		int stringlen = LS3_LENGTH_100 * 3;
		char sql[stringlen];
		memset(sql, 0, stringlen * sizeof(char));

		strncpy(sql, LS3_QGET_FNME, stringlen);
		sql[stringlen] = 0;

		replace(sql, "&fnme", _filename);

//		writeLogFile("", sql);

		if (get_db_data(sql, fname, NULL, NULL) != LS3_OK)
			return LS3_OK;

		if (fname[0] == 0)
			crypto_filename(_filename, fname, NULL);

		strncpy(fname, concat(_socpath, fname),
				strlen(_socpath) + strlen(fname));
		/* 1. compare hash backup file with db, pHash are now contain dbhash */
		if (compare_hash(fname, dbhash) != LS3_OK) {
			writeLogFile("", "Backup file corrupted!\nRestore sequence terminated!");
			return LS3_ER;
		} else
			writeLogFile("", "Backup file clear!\nRestoring file...");

		/* 2. restore backup file to destination
		 * since _filename contain destination path,
		 * we can extract destination path information from them :D*/
		if (copy_file2(fname, _filename) != LS3_OK)
			return LS3_ER;

		/* 3. verify restore file hash with db hash
		 * 3.1. calculate hash restored file */
		if (get_file_hash(_filename, pHash) != LS3_OK) {
			writeLogFile("", concat(concat("cannot calculate file ", _filename), "\nRestore sequence terminated!"));
			return LS3_ER;
		}

		/* assigning dbhash to pHash */

		writeLogFile("", "Verifying restored file...");

		if (get_db_hash(_filename, dbhash, NULL, NULL) != LS3_OK)
			return LS3_ER;

		if (strncmp(pHash, dbhash, B5_SHA256_BLOCK_SIZE) != LS3_OK) {
			/* 4. if not match, repeat restore sequence (3 trials max) */
			if (trial > 0) {
				writeLogFile("", "Hash file not match!, repeating copy...");
				Sleep(2000);
				trial--;
				goto repeat_copy;
			}

			writeLogFile("", "Reaching maximum trial, copy file failed!\nRestore sequence terminated!");

			return LS3_ER;

		} else {
			update_mod_file(_filename, get_unixepoch_time(lLupd));
		}

		memset(fname, 0, LS3_LENGTH_DEV_PATH_1 * sizeof(char));
		memset(sql, 0, stringlen * sizeof(char));

	} else {
		writeLogFile("", concat(_filename, " Clean!"));
	}

	writeLogFile("", "Verified OK!");

	memset(dbhash, 0, sizeof dbhash);
	memset(pLupd, 0, sizeof pLupd);
	memset(pHash, 0, sizeof pHash);
	memset(dbfsize, 0, sizeof dbfsize);
	memset(filename, 0, sizeof filename);
	memset(lLupd, 0, sizeof lLupd);

	return LS3_OK;
}

int integrity_test_2(char* _dirname, char* _socpath, int _restore, int _rec) {
	DIR *d, *sd;
	struct dirent *dir;
	char bd[LS3_LENGTH_256];
	int len;

	/* phase 1: checking existing files */

	d = opendir(_dirname);

	if (d) {
		while ((dir = readdir(d)) != NULL) {

			len = strlen(dir->d_name);

			if (strncmp(dir->d_name, ".", len) != 0
					&& strncmp(dir->d_name, "..", len) != 0) {

				memset(bd, 0, LS3_LENGTH_256 * sizeof(char));
				sprintf(bd, "%s\\\%s", _dirname, dir->d_name);

				sd = opendir(bd);

				if (sd) {
					if (_rec==1)
						integrity_test_2(bd, _socpath, _restore, _rec);

					closedir(sd);

					//rmdir(bd);

				} else {
					integrity_test(bd, _socpath, _restore);
				}
			}

		}
		closedir(d);
	}

	if(_restore==1){

		/* phase 2: to restore what files should exists */

		len = strlen(LS3_QGET_FILE_BY_ID);
		char id[LS3_LENGTH_4];
		char source[LS3_LENGTH_500];
		char sql[LS3_LENGTH_1000];

		memset(id, 0, LS3_LENGTH_4 * sizeof(char));

		id[0] = (char) 48;

		loop:

		memset(source, 0, LS3_LENGTH_500 * sizeof(char));
		memset(sql, 0, LS3_LENGTH_1000 * sizeof(char));
		memcpy(sql, LS3_QGET_FILE_BY_ID, len);
		sql[len] = 0;

		replace(sql, "&dsph", _dirname);
		replace(sql, "&id", id);

		if (LS3_OK == get_db_data(sql, id, source, NULL)) {

			integrity_test(source, _socpath, _restore);

			goto loop;

		}

		memset(id, 0, sizeof id);
		memset(source, 0, sizeof source);
		memset(sql, 0, sizeof sql);
	}



	return LS3_OK;
}

int create_rec_dir(char* _dir) {

	DIR* dir;
	int len = strlen(LS3_QGET_DLIST);
	char id[LS3_LENGTH_4];
	char source[LS3_LENGTH_500];
	char sql[LS3_LENGTH_1000];

	memset(id, 0, LS3_LENGTH_4 * sizeof(char));

	id[0] = (char) 48;

	loop:

	memset(source, 0, LS3_LENGTH_500 * sizeof(char));
	memset(sql, 0, LS3_LENGTH_1000 * sizeof(char));
	memcpy(sql, LS3_QGET_DLIST, len);
	sql[len] = 0;

	replace(sql, "&dsph", concat(_dir, "\\%"));
	replace(sql, "&id", id);

//	writeLogFile("",sql);

	if (LS3_OK == get_db_data(sql, id, source, NULL)) {

		dir = opendir(source);

		if (dir) {
			/* Directory exists. */
			closedir(dir);
		} else {
			/* Directory does not exist. */
			system(concat("mkdir ", source));
		}

		goto loop;

	}

	memset(id, 0, sizeof id);
	memset(source, 0, sizeof source);
	memset(sql, 0, sizeof sql);

	return LS3_OK;
}

int copy_file(char *_source, char *_dest) {

	FILE *source, *target;

	char ch;
	source = fopen(_source, "r");

	if (source == NULL) {
		writeLogFile("", "Cannot open source file...");
		return LS3_ER;
	}

	target = fopen(_dest, "w");

	if (target == NULL) {
		fclose(source);
		writeLogFile("", "Cannot write file to destination...");
		return LS3_ER;
	}

	while ((ch = fgetc(source)) != EOF)
		fputc(ch, target);

	fclose(source);
	fclose(target);

	return LS3_OK;
}

int copy_file2(char *old_filename, char *new_filename) {
	FILE *ptr_old, *ptr_new;
	errno_t err = 0, err1 = 0;
	int a;

	err = fopen_s(&ptr_old, old_filename, "rb");
	err1 = fopen_s(&ptr_new, new_filename, "wb");

	if (err != 0)
		return -1;

	if (err1 != 0) {
		fclose(ptr_old);
		return -1;
	}

	while (1) {
		a = fgetc(ptr_old);

		if (!feof(ptr_old))
			fputc(a, ptr_new);
		else
			break;
	}

	fclose(ptr_new);
	fclose(ptr_old);
	return 0;
}

char* get_modified_file(char* _filepath, int _opt) {
	struct tm *foo;
	struct stat attrib;

	stat(_filepath, &attrib);
	foo = localtime(&(attrib.st_mtime));
	char *buff = malloc(26);
	strftime(buff, 26, (_opt == 0) ? "%Y-%m-%d %H:%M:%S" : "%Y%m%d%H%M%S", foo);

	return buff;
}

long get_file_size(char* _filepath) {
	struct stat attrib;

	stat(_filepath, &attrib);

	return attrib.st_size;
}

void update_mod_file(char* _fname, time_t _epoch) {
	struct _utimbuf ut;
	ut.actime = _epoch;
	ut.modtime = _epoch;
	_utime(_fname, &ut);
}

int set_file_readonly(char* _fname, int _param) {

	if (_param == 0)
		/* remove readonly */
		return system(concat("attrib -r ", _fname));
	else
		/*set readonly*/
		return system(concat("attrib +r ", _fname));
}

void file_list(char *path) {
	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			printf("%s\n", dir->d_name);
		}
		closedir(d);
	}
}
