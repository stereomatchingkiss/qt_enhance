#include "download_supervisor.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QNetworkAccessManager>

namespace qte{

namespace net{

download_supervisor::download_supervisor(QObject *parent)
    : QObject(parent),
      max_download_file_(1),
      network_access_(new QNetworkAccessManager(this)),
      total_download_file_(0),
      unique_id_(0)
{     
}

size_t download_supervisor::append(const QNetworkRequest &request, const QString &save_at, bool save_as_file)
{
    auto task = std::make_shared<download_task>();
    task->unique_id_ = unique_id_++;
    task->network_request_ = request;
    task->save_at_ = save_at;
    task->save_as_file_ = save_as_file;
    id_table_.insert({task->unique_id_, task});
    download_start(task);

    return task->unique_id_;
}

size_t download_supervisor::get_max_download_file() const
{
    return max_download_file_;
}

void download_supervisor::set_max_download_file(size_t val)
{
    max_download_file_ = val;
}

void download_supervisor::start_next_download()
{
    //TODO : use hint to speed things up
    if(!id_table_.empty()){
        auto it = std::begin(id_table_);
        for(; it != std::end(id_table_); ++it){
            if(it->second->network_error_code_ == QNetworkReply::NoError &&
                    it->second->file_can_open_){
                download_start(it->second);
                break;
            }
        }
        if(it == std::end(id_table_)){
            //TODO : emit signal to tell the slot every download are done
            emit all_download_finished();
        }
    }else{
        emit all_download_finished();
    }
}

void download_supervisor::process_download_finished()
{
    if(total_download_file_ > 0){
        --total_download_file_;
    }
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto rit = reply_table_.find(reply);
        if(rit != std::end(reply_table_)){
            auto const unique_id = rit->second->unique_id_;
            auto const download_data = rit->second->data_;
            reply_table_.erase(rit);
            auto id_it = id_table_.find(unique_id);
            if(id_it != std::end(id_table_)){
                id_table_.erase(id_it);
            }
            emit download_finished(unique_id, download_data);
            start_next_download();
        }
        if(reply->error() != QNetworkReply::NoError){
            qDebug()<<"download error:"<<reply->errorString();
        }
    }else{
        qDebug()<<__func__<<":QNetworkReply is nullptr";
    }
}

void download_supervisor::error_handle(QNetworkReply::NetworkError)
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto it = reply_table_.find(reply);
        if(it != std::end(reply_table_)){
            emit error(it->second->unique_id_, reply->errorString());
        }
    }
}

void download_supervisor::handle_download_progress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug()<<__func__<< " receive "<<bytesReceived;
    qDebug()<<__func__<< " total "<<bytesTotal;
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto rit = reply_table_.find(reply);
        if(rit != std::end(reply_table_)){
            emit download_progress(rit->second->unique_id_, bytesReceived,
                                   bytesTotal);
        }
    }
}

void download_supervisor::ready_read()
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto it = reply_table_.find(reply);
        if(it != std::end(reply_table_)){
            QByteArray data(reply->bytesAvailable(), Qt::Uninitialized);
            if(it->second->save_as_file_ && it->second->file_.isOpen()){
                reply->read(data.data(), data.size());
                it->second->file_.write(data);
            }else{
                it->second->data_ += data;
            }
        }
    }else{
        qDebug()<<__func__<< ":reply is nullptr or fail to cast from sender()";
    }
}

void download_supervisor::launch_download_task(std::shared_ptr<download_supervisor::download_task> &task)
{
    ++total_download_file_;
    task->network_reply_ = network_access_->get(task->network_request_);
    reply_table_.insert({task->network_reply_, task});
    connect(task->network_reply_, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &download_supervisor::error_handle);
    connect(task->network_reply_, &QNetworkReply::readyRead, this, &download_supervisor::ready_read);
    connect(task->network_reply_, static_cast<void(QNetworkReply::*)()>(&QNetworkReply::finished),
            this, &download_supervisor::process_download_finished);
    connect(task->network_reply_, &QNetworkReply::downloadProgress, this, &download_supervisor::handle_download_progress);
}

void download_supervisor::download_start(std::shared_ptr<download_task> &task)
{
    if(total_download_file_ < max_download_file_){
        if(task->save_as_file_){
            qDebug()<<task->save_at_ + "/" + save_file_name(*task);
            task->file_.setFileName(task->save_at_ + "/" + save_file_name(*task));
            if(task->file_.open(QIODevice::WriteOnly)){
                launch_download_task(task);
            }else{
                task->file_can_open_ = false;
                emit error(task->unique_id_, tr("Cannot open file"));
            }
        }else{
            launch_download_task(task);
        }
    }
}

QString download_supervisor::save_file_name(const download_supervisor::download_task &task) const
{
    QFileInfo file_info(task.get_url().path());
    QString file_name = file_info.fileName();
    if(QFile::exists(task.save_at_ + "/" + file_name)){
        QString const base_name = file_info.baseName();
        QString complete_suffix = file_info.completeSuffix();
        if(complete_suffix.isEmpty()){
            complete_suffix = "txt";
        }
        QString new_file_name = base_name + "(0)." + complete_suffix;
        for(size_t i = 1; QFile::exists(task.save_at_ + "/" + new_file_name); ++i){
            new_file_name = base_name + "(" + QString::number(i) + ")." + complete_suffix;
        }

        return new_file_name;
    }

    return file_name;
}

size_t download_supervisor::download_task::get_unique_id() const
{
    return unique_id_;
}

QUrl download_supervisor::download_task::get_url() const
{
    return network_request_.url();
}

} //namespace net

} //namespace qte
