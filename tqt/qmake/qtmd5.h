#ifndef TQTMD5_H
#define TQTMD5_H
#include <tqstring.h>
#include <tqcstring.h>

void qtMD5(const TQByteArray &src, unsigned char *digest);
TQString qtMD5(const TQByteArray &src);

#endif


