#include "file_compressor.hpp"

#include <QByteArray>
#include <QFile>

namespace qte{

namespace cp{

void compress(QString const &file_name, QString const &compress_file_name)
{
    QFile infile(file_name);
    QFile outfile(compress_file_name);
    infile.open(QIODevice::ReadOnly);
    outfile.open(QIODevice::WriteOnly);
    QByteArray uncompressed_data = infile.readAll();
    QByteArray compressed_data = qCompress(uncompressed_data, 9);
    outfile.write(compressed_data);
}

void decompress(QString const &file_name, QString const &decompress_file_name)
{
    QFile infile(file_name);
    QFile outfile(decompress_file_name);
    infile.open(QIODevice::ReadOnly);
    outfile.open(QIODevice::WriteOnly);
    QByteArray uncompressed_data = infile.readAll();
    QByteArray compressed_data = qUncompress(uncompressed_data);
    outfile.write(compressed_data);
}

}}	
