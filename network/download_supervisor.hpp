#ifndef QTE_NET_DOWNLOAD_SUPERVISOR_HPP
#define QTE_NET_DOWNLOAD_SUPERVISOR_HPP

#include <QFile>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QUrl>

#include <map>
#include <memory>

class QNetworkAccessManager;

namespace qte{

namespace net{

/**
 * Manage multiple download files, similar to download_manager of
 * qt_enhance, but this class only depend on Qt5 and standard c++
 */
class download_supervisor : public QObject
{
    Q_OBJECT
public:
    struct download_task
    {
        friend class download_supervisor;

        QString const& get_error_string() const;
        QNetworkReply::NetworkError get_network_error_code() const;
        QString const& get_save_at() const;
        QString get_save_as() const;
        bool get_is_timeout() const;
        size_t get_unique_id() const;
        QUrl get_url() const;        

    private:
        QByteArray data_;
        QString error_string_;
        QFile file_;
        bool file_can_open_ = true;
        bool is_timeout_ = false;
        QNetworkReply::NetworkError network_error_code_ = QNetworkReply::NoError;
        QNetworkReply *network_reply_ = nullptr;
        QNetworkRequest network_request_;
        QString save_at_;
        bool save_as_file_ = true;
        QTimer timer_;
        int timeout_msec_ = -1;
        size_t unique_id_ = 0;
    };

    explicit download_supervisor(QObject *parent = nullptr);

    /**
      * Append new data to the download list, this would start download task,
      * call start_download_task to start download
      * @param request url of the data want to download
      * @param save_at the location you want to save the file at      
      * @return unique id for each download request
      */
    size_t append(QNetworkRequest const &request, QString const &save_at);

    /**
     * @brief overload of append, this function will save the download data
     * in QByteArray rather than write them to a file
     */
    size_t append(QNetworkRequest const &request);

    /**
     * @brief Overload of append, everything are same except this function can
     * specify timeout_msec
     * @param timeout_msec If the network request do not have any progress within
     * timeout_msec, the network request will be aborted
     */
    size_t append(QNetworkRequest const &request, QString const &save_at,
                  int timeout_msec);

    /**
     * @brief overload of append, this function will save the download data
     * in QByteArray rather than write them to a file
     */
    size_t append(QNetworkRequest const &request, int timeout_msec);

    QNetworkAccessManager* get_network_manager() const;

    /**
      * This value determine how many items could be downloaded
      * at the same time
      * @return maximum download size
      */
    size_t get_max_download_file() const;

    /**
      * Set maximum download size
      * @param maximum download size
      */
    void set_max_download_file(size_t val);

    void set_proxy(QNetworkProxy const &proxy);

    /**
     * @brief start to download if unique id exist in task list
     * @param unique_id self explained
     */
    void start_download_task(size_t unique_id);

signals:
    void all_download_finished();
    void download_finished(std::shared_ptr<download_task> task);
    void download_progress(std::shared_ptr<download_task> task, qint64 bytesReceived, qint64 bytesTotal);

    void error(std::shared_ptr<download_task> task, QString const &error_msg);

private:
    size_t append(QNetworkRequest const &request, QString const &save_at, int timeout_msec, bool save_as_file);
    void download_start(std::shared_ptr<download_task> task);
    void handle_download_finished();
    void handle_download_progress(qint64 bytesReceived, qint64 bytesTotal);
    void handle_error(QNetworkReply::NetworkError code);
    void handle_ready_read();
    void launch_download_task(std::shared_ptr<download_task> task);
    void restart_timer(download_task &task);    
    void start_next_download();    

    std::map<size_t, std::shared_ptr<download_task>> id_table_;
    size_t max_download_file_;
    QNetworkAccessManager *network_access_;
    std::map<QNetworkReply*, std::shared_ptr<download_task>> reply_table_;
    size_t total_download_file_;
    size_t unique_id_;
};

} //namespace net

} //namespace qte

#endif // QTE_NET_DOWNLOAD_SUPERVISOR_HPP
