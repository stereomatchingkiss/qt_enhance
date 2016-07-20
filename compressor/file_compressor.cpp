#include "file_compressor.hpp"

#include <QByteArray>
#include <QFile>

namespace qte{

namespace cp{

bool compress(QString const &file_name, QString const &compress_file_name)
{    
    QFile infile(file_name);
    QFile outfile(compress_file_name);
    bool can_open = infile.open(QIODevice::ReadOnly);
    if(!can_open){
        return false;
    }
    can_open = outfile.open(QIODevice::WriteOnly);
    if(!can_open){
        return false;
    }
    QByteArray uncompressed_data = infile.readAll();
    QByteArray compressed_data = qCompress(uncompressed_data, 9);
    outfile.write(compressed_data);
}

bool decompress(QString const &file_name, QString const &decompress_file_name)
{
    QFile infile(file_name);
    QFile outfile(decompress_file_name);
    bool can_open = infile.open(QIODevice::ReadOnly);
    if(!can_open){
        return false;
    }
    can_open = outfile.open(QIODevice::WriteOnly);
    if(!can_open){
        return false;
    }
    QByteArray uncompressed_data = infile.readAll();
    QByteArray compressed_data = qUncompress(uncompressed_data);
    outfile.write(compressed_data);
}

}}	
