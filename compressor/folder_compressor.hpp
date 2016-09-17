#ifndef FOLDERCOMPRESSOR_H
#define FOLDERCOMPRESSOR_H

#include <QDataStream>
#include <QDir>
#include <QFile>

namespace qte{

namespace cp{

/**
 * @brief This class come from http://3adly.blogspot.my/2011/06/qt-folder-compression.html
 */
class folder_compressor
{    
public:
    folder_compressor();

    //A recursive function that scans all files inside the source folder
    //and serializes all files in a row of file names and compressed
    //binary data in a single file
    bool compress_folder(QString const &sourceFolder, QString const &destinationFile,
                         int compression_level = 9);
    bool compress_folder(QString const &sourceFolder, QString const &destinationFile,
                         QStringList const &exclude_content,
                         int compression_level = 9);

    //A function that deserializes data from the compressed file and
    //creates any needed subfolders before saving the file
    bool decompress_folder(QString const &sourceFile, QString const &destinationFolder);

private:
    QFile file;
    QDataStream dataStream;

    bool compress(QString const &sourceFolder, QString const &prefex,
                  int compression_level);
    bool compress(QString const &sourceFolder, QString const &prefex,
                  QStringList const &exclude_content,
                  int compression_level);
};

}}

#endif // FOLDERCOMPRESSOR_H
