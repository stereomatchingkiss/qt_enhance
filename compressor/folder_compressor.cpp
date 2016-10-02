#include "folder_compressor.hpp"

#include <set>

namespace qte{

namespace cp{

folder_compressor::folder_compressor()
{
}

bool folder_compressor::compress_folder(QString const &sourceFolder,
                                        QString const &destinationFile,
                                        int compression_level)
{
    return compress_folder(sourceFolder, destinationFile, {}, compression_level);
}

bool folder_compressor::compress_folder(const QString &sourceFolder,
                                        const QString &destinationFile,
                                        const QStringList &exclude_content,
                                        int compression_level)
{
    QDir src(sourceFolder);
    if(!src.exists())//folder not found
    {
        return false;
    }

    file_.setFileName(destinationFile);
    if(!file_.open(QIODevice::WriteOnly))//could not open file
    {
        return false;
    }

    data_stream_.setDevice(&file_);

    bool success = compress(sourceFolder, "", exclude_content, compression_level);
    file_.close();

    return success;
}

bool folder_compressor::compress(QString const &sourceFolder,
                                 QString const &prefex,
                                 int compression_level)
{
    return compress(sourceFolder, prefex, {}, compression_level);
}

bool folder_compressor::compress(const QString &sourceFolder,
                                 const QString &prefex,
                                 const QStringList &exclude_content,
                                 int compression_level)
{
    QDir dir(sourceFolder);
    if(!dir.exists())
        return false;

    //1 - list all folders inside the current folder
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
    QFileInfoList foldersList = dir.entryInfoList();

    std::set<QString> exclude_set(std::begin(exclude_content),
                                  std::end(exclude_content));
    //2 - For each folder in list: call the same function with folders' paths
    for(int i = 0; i < foldersList.length(); ++i)
    {
        QString const folderName = foldersList.at(i).fileName();
        if(exclude_set.find(folderName) == std::end(exclude_set)){
            QString const folderPath = dir.absolutePath() + "/" + folderName;
            QString const newPrefex = prefex + "/" + folderName;
            compress(folderPath, newPrefex, compression_level);
        }
    }

    //3 - List all files inside the current folder
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
    QFileInfoList filesList = dir.entryInfoList();

    //4- For each file in list: add file path and compressed binary data
    for(int i=0; i<filesList.length(); i++)
    {
        QFile file(dir.absolutePath()+"/"+filesList.at(i).fileName());
        if(!file.open(QIODevice::ReadOnly))//couldn't open file
        {
            return false;
        }

        data_stream_ << QString(prefex+"/"+filesList.at(i).fileName());
        data_stream_ << qCompress(file.readAll(), compression_level);
    }

    return true;
}

bool folder_compressor::decompress_folder(QString const &sourceFile,
                                          QString const &destinationFolder)
{
    //validation
    QFile src(sourceFile);
    if(!src.exists())
    {//file not found, to handle later
        return false;
    }
    QDir dir;
    if(!dir.mkpath(destinationFolder))
    {//could not create folder
        return false;
    }

    file_.setFileName(sourceFile);
    if(!file_.open(QIODevice::ReadOnly))
        return false;

    data_stream_.setDevice(&file_);

    while(!data_stream_.atEnd())
    {
        QString fileName;
        QByteArray data;

        //extract file name and data in order
        data_stream_ >> fileName >> data;

        //create any needed folder
        QString subfolder;
        for(int i=fileName.length()-1; i>0; i--)
        {
            if((QString(fileName.at(i)) == QString("\\")) || (QString(fileName.at(i)) == QString("/")))
            {
                subfolder = fileName.left(i);
                dir.mkpath(destinationFolder+"/"+subfolder);
                break;
            }
        }

        QFile outFile(destinationFolder+"/"+fileName);
        if(!outFile.open(QIODevice::WriteOnly))
        {
            file_.close();
            return false;
        }
        outFile.write(qUncompress(data));
        outFile.close();
    }

    file_.close();
    return true;
}

}}
