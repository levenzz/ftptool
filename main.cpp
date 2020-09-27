#include <ftptool.h>

void main() {
	ftp_upload_file("127.0.0.1",21,"saa","123456",file_name.c_str(), "1.jpg", "");
}