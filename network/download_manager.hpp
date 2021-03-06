#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include "download_info.hpp"

#include <QObject>
#include <QNetworkReply>
#include <QStringList>
#include <QUrl>

class QNetworkAccessManager;

namespace qte{

namespace net{

/**
 * Manage multiple download files, by now only support/test
 * http request
 */
class download_manager : public QObject
{
    Q_OBJECT
public:
    explicit download_manager(QObject *obj = nullptr);

    /**
     * Append new data to the download list, you should
     * call the api "start_download" to start the
     * appended download request but not call the append
     * api again, because append api will add new download
     * request into the download list rather than
     * start the download progress of appended request
     * @param url url of the data want to download
     * @param save_at the location you want to save the file at
     * @param save_as the file name of the download target
     * @return unique id for each download request, value >= 0
     * indicate append operation success and vice versa
     */
    int_fast64_t append(QUrl const &url,
                        QString const &save_at,
                        QString const &save_as);

    /**
     * Overload of append(url, save_at, save_as), different part
     * is it will save the download data into QByteArray but
     * not file
     * @param url url of the data want to download
     * @return unique id for each download request, value >= 0
     * indicate append operation success and vice versa
     */
    int_fast64_t append(QUrl const &url);

    /**
     * Clear all of the request wait to download
     * or downloaded
     */
    void clear_download_list();

    /**
     * Erase the download item
     * @param uuid the unique id of the item want to delete
     * @return true if the request can be erased and vice versa
     */
    bool erase(int_fast64_t uuid);

    /**
     * Get the maximum download size of download manager,
     * this value determine how many items could be downloaded
     * at the same time
     * @return maximum download size
     */
    size_t get_max_download_size() const;

    /**
     * Return how many files are downloading
     * @return size of downloading files
     */
    size_t get_total_download_file() const;

    /**
     * restart the download request
     * @param uuid unique id of the request
     * @return true if able to restart, else false
     */
    bool restart_download(int_fast64_t uuid);

    /**
     * Restart(new) QNetworkAccessManger because Qt5 got some bugs--
     * Once network error occur, it may not be able to download anything
     * @return
     */
    bool restart_network_manager();

    /**
     * Set maximum download size, this value determine how
     * many items could be downloaded at the same time
     * @param value value of maximum download size
     */
    void set_max_download_size(size_t value);

    /**
     * start the download in the download list, every
     * download has it associated unique id. If the file already
     * downloaded by append api, the uuid will not exist. The purpose
     * of this api is redownload the file cannot download due to some
     * errors.
     * @param uuid the unique id of the item want to download
     * @return true if the download can begin and vice versa
     */
    bool start_download(int_fast64_t uuid);

signals:
    /**
     * emit when there are download error happened
     * @param uuid unique id of the download request
     * @param error error message of download
     */
    void download_error(int_fast64_t uuid, QString error);
    /**
     * emit when download finished
     * @param uuid unique id of the download request
     * @param data data download from the url, only available if
     * the save_at || save_as is empty
     * @param error error message of download, will be empty
     * if download operation has no error
     */
    void download_finished(int_fast64_t uuid, QByteArray data,
                           QString error);
    /**
     * emitted to indicate the progress of the download part of this
     * network request, if there's any. If there's no download
     * associated with this request, this signal will be emitted
     * once with 0 as the value of both bytesReceived and bytesTotal.
     * @param uuid unique id of the download request
     * @param bytes_received bytesReceived parameter indicates the number
     * of bytes received
     * @param bytes_total indicates the total number
     * of bytes expected to be downloaded. If the number of bytes to
     * be downloaded is not known, bytesTotal will be -1.
     */
    void download_progress(int_fast64_t uuid,
                           qint64 bytes_received,
                           qint64 bytes_total);
    /**
     * Same as the readyRead() of QIODevice, but it will
     * pass with the uuid of the request
     * @param uuid unique id of the download request
     */
    void download_ready_read(int_fast64_t uuid);
    /**
     * emit when the downloading item decrease
     * @param value number of the downloading item
     */
    void downloading_size_decrease(size_t value);

private slots:        
    void download_finished();
    void download_progress(qint64 bytes_received,
                           qint64 bytes_total);
    void download_ready_read();

    void error(QNetworkReply::NetworkError code);

private:
    void connect_network_reply(QNetworkReply *reply,
                               bool is_connect = true);

    int_fast64_t append_impl(QUrl const &value,
                             QString const &save_at,
                             QString const &save_as);

    download_info_index download_info_;
    QNetworkAccessManager *manager_;
    size_t max_download_size_;
    size_t total_download_files_;
    int_fast64_t uuid_;
};

}

}

#endif // DOWNLOAD_MANAGER_H
