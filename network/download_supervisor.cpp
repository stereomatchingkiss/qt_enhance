#include "download_supervisor.hpp"

#include <QDebug>
#include <QFileDevice>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QRegularExpression>

#include <functional>

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

size_t download_supervisor::append(const QNetworkRequest &request, const QString &save_at)
{    
    return append(request, save_at, -1, true);
}

size_t download_supervisor::append(const QNetworkRequest &request)
{
    return append(request, "", -1, false);
}

size_t download_supervisor::append(const QNetworkRequest &request, const QString &save_at,
                                   int timeout_msec)
{     
     return append(request, save_at, timeout_msec, true);
}

size_t download_supervisor::append(const QNetworkRequest &request, int timeout_msec)
{
    return append(request, "", timeout_msec, false);
}

QNetworkAccessManager* download_supervisor::get_network_manager() const
{
    return network_access_;
}

size_t download_supervisor::get_max_download_file() const
{
    return max_download_file_;
}

void download_supervisor::set_max_download_file(size_t val)
{
    max_download_file_ = val;
}

void download_supervisor::set_proxy(const QNetworkProxy &proxy)
{
    network_access_->setProxy(proxy);
}

void download_supervisor::start_download_task(size_t unique_id)
{
    auto it = id_table_.find(unique_id);
    if(it != std::end(id_table_)){
        download_start(it->second);
    }
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
            emit all_download_finished();
        }
    }else{
        emit all_download_finished();
    }
}

void download_supervisor::handle_download_finished()
{
    if(total_download_file_ > 0){
        --total_download_file_;
    }
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto rit = reply_table_.find(reply);
        if(rit != std::end(reply_table_)){
            auto task = rit->second;            
            task->timer_.stop();
            task->file_.close();
            if(reply->error() != QNetworkReply::NoError){
                task->network_error_code_ = reply->error();
                if(task->error_string_.isEmpty()){
                    task->error_string_ = reply->errorString();
                }
            }
            auto const unique_id = task->unique_id_;            
            reply_table_.erase(rit);
            auto id_it = id_table_.find(unique_id);            
            if(id_it != std::end(id_table_)){
                id_table_.erase(id_it);
            }            
            emit download_finished(task);
            start_next_download();
        }        
    }else{
        qDebug()<<__func__<<":QNetworkReply is nullptr";
    }
}

void download_supervisor::handle_error(QNetworkReply::NetworkError)
{
    auto *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply){
        auto it = reply_table_.find(reply);
        if(it != std::end(reply_table_)){
            it->second->error_string_ = reply->errorString();
            emit error(it->second, reply->errorString());
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
            restart_timer(*rit->second);
            emit download_progress(rit->second, bytesReceived,
                                   bytesTotal);
        }
    }
}

void download_supervisor::handle_ready_read()
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

size_t download_supervisor::append(const QNetworkRequest &request, const QString &save_at,
                                   int timeout_msec, bool save_as_file)
{
    auto task = std::make_shared<download_task>();
    task->unique_id_ = unique_id_++;
    task->network_request_ = request;
    task->save_at_ = save_at;
    task->save_as_file_ = save_as_file;
    task->timeout_msec_ = timeout_msec;
    id_table_.insert({task->unique_id_, task});

    return task->unique_id_;
}

void download_supervisor::launch_download_task(std::shared_ptr<download_supervisor::download_task> task)
{
    ++total_download_file_;
    task->network_reply_ = network_access_->get(task->network_request_);    
    restart_timer(*task);
    reply_table_.insert({task->network_reply_, task});
    connect(&task->timer_, &QTimer::timeout, task->network_reply_, [this, task]()
    {
        task->timer_.stop();
        task->is_timeout_ = true;
        task->network_reply_->abort();
    });
    connect(task->network_reply_, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &download_supervisor::handle_error);
    connect(task->network_reply_, &QNetworkReply::readyRead, this, &download_supervisor::handle_ready_read);
    connect(task->network_reply_, static_cast<void(QNetworkReply::*)()>(&QNetworkReply::finished),
            this, &download_supervisor::handle_download_finished);
    connect(task->network_reply_, &QNetworkReply::downloadProgress, this, &download_supervisor::handle_download_progress);
    connect(task->network_reply_, static_cast<void(QNetworkReply::*)()>(&QNetworkReply::finished),
            task->network_reply_, &QNetworkReply::deleteLater);
}

void download_supervisor::restart_timer(download_supervisor::download_task &task)
{
    if(task.timeout_msec_ > 0){
        task.timer_.start(task.timeout_msec_);
    }
}

void download_supervisor::download_start(std::shared_ptr<download_task> task)
{
    if(total_download_file_ < max_download_file_){
        if(task->save_as_file_){
            qDebug()<<__func__<<":"<<task->save_at_ + "/" + save_file_name(*task);
            task->file_.setFileName(task->save_at_ + "/" + save_file_name(*task));
            if(task->file_.open(QIODevice::WriteOnly)){
                launch_download_task(task);
            }else{                
                task->file_can_open_ = false;
                task->error_string_ = tr("Cannot open file %1").arg(task->file_.fileName());
                emit error(task, task->error_string_);
                emit download_finished(task);
            }
        }else{
            launch_download_task(task);
        }
    }
}

QString download_supervisor::save_file_name(const download_supervisor::download_task &task) const
{
    QFileInfo file_info(task.get_url().toString());
    QRegularExpression const re("[<>:\"/\\*\\?\\|\\\\]");
    QString file_name = file_info.fileName().remove(re).trimmed();
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

        return new_file_name.trimmed();
    }

    return file_name;
}

const QString &download_supervisor::download_task::get_error_string() const
{
    return error_string_;
}

QNetworkReply::NetworkError download_supervisor::download_task::get_network_error_code() const
{
    return network_error_code_;
}

QString const& download_supervisor::download_task::get_save_at() const
{
    return save_at_;
}

QString download_supervisor::download_task::get_save_as() const
{
    return file_.fileName();
}

bool download_supervisor::download_task::get_is_timeout() const
{
    return is_timeout_;
}

size_t download_supervisor::download_task::get_unique_id() const
{
    return unique_id_;
}

QUrl download_supervisor::download_task::get_url() const
{
    return network_request_.url();
}

bool download_supervisor::download_task::remove_file()
{
    return file_.remove();
}

} //namespace net

} //namespace qte
