#ifndef FOLDERCOMPRESSOR_H
#define FOLDERCOMPRESSOR_H

#include <QFile>
#include <QDir>

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
    bool compress_folder(QString const &sourceFolder, QString const &destinationFile);

    //A function that deserializes data from the compressed file and
    //creates any needed subfolders before saving the file
    bool decompress_folder(QString const &sourceFile, QString const &destinationFolder);

private:
    QFile file;
    QDataStream dataStream;

    bool compress(QString const &sourceFolder, QString const &prefex);
};

}}

#endif // FOLDERCOMPRESSOR_H
