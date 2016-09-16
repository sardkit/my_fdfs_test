#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fdfs_client.h>
#include "make_log.h"


int main(int argc, char *argv[])
{
	char *conf_filename;
	char *local_filename;
	int result;
	ConnectionInfo *pTrackerServer;
	ConnectionInfo storageServer;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	int store_path_index;
	char file_id[128];

	if(argc < 3)
	{
		printf("Usage: %s <config_file> <local_filename>\n", argv[0]);
		return 1;
	}
	if(argc > 3)
		printf("Too many parameters");

	conf_filename = argv[1];
	result = fdfs_client_init(conf_filename);
	if(result != 0)
	{
		LOG("Test01", "My_upload_file", "fdfs_client_init error; result: %d", result);
		return result;
	}

	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		LOG("Test01", "My_upload_file", "tracker_get_connection error;");
		return -1;
	}

	*group_name = '\0';
	store_path_index = 0;
	if ((result=tracker_query_storage_store(pTrackerServer, \
					&storageServer, group_name, &store_path_index)) != 0)
	{
		fdfs_client_destroy();
		LOG("Test01", "My_upload_file", "tracker_query_storage fail; result: %d", result);
		return result;
	}

	local_filename = argv[2];
	result = storage_upload_appender_by_filename1(pTrackerServer, \
			&storageServer, store_path_index, \
			local_filename, NULL, \
			NULL, 0, group_name, file_id);
	if (result != 0)
	{ 
		LOG("Test01", "My_upload_file", "upload file fail; result: %d", result);

		tracker_disconnect_server_ex(pTrackerServer, true);
		fdfs_client_destroy();
		return result;
	}
	printf("%s\n", file_id);
	LOG("Test01", "My_upload_file", "result: %s", file_id);

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();


	return 0;
}
