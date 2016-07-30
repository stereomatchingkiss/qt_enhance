#include <QString>

/**
 *The codes come from http://www.antonioborondo.com/2014/10/22/zipping-and-unzipping-files-with-qt/
 */

namespace qte{

namespace cp{

bool compress(QString const &file_name, QString const &compress_file_name,
               int compressionLevel = 9);
bool decompress(QString const &file_name, QString const &decompress_file_name);
	
}
}	
