#ifndef SF1R_NODEMANAGER_ASYNCLOGMGR_H
#define SF1R_NODEMANAGER_ASYNCLOGMGR_H

#include "DistributeFileSyncRequest.h"
#include <string>
#include <util/singleton.h>
#include <3rdparty/msgpack/rpc/server.h>
#include <vector>
#include <boost/thread.hpp>
#include <boost/threadpool.hpp>

namespace sf1r
{

class RpcServerConnection;

class DistributeFileSyncMgr
{
public:
    static DistributeFileSyncMgr* get()
    {
        return ::izenelib::util::Singleton<DistributeFileSyncMgr>::get();
    }

    DistributeFileSyncMgr();
    ~DistributeFileSyncMgr();
    void init();
    void stop();
    bool getNewestReqLog(uint32_t start_from, std::vector<std::string>& saved_log);
    bool syncNewestSCDFileList(const std::string& colname);
    bool syncCollectionData(const std::string& colname);
    bool getFileFromOther(const std::string& filepath, bool force_overwrite = false);
    void notifyFinishReceive(const std::string& filepath);
    void sendFinishNotifyToReceiver(const std::string& ip, uint16_t port, const FinishReceiveRequest& req);
    bool pushFileToAllReplicas(const std::string& srcpath, const std::string& destpath, bool recrusive = false);

    void checkReplicasStatus(const std::string& colname, std::string& check_errinfo);
    void checkReplicasLogStatus(std::string& check_errinfo);
    void notifyReportStatusRsp(const ReportStatusRspData& rspdata);
    void sendReportStatusRsp(const std::string& ip, uint16_t port, const ReportStatusRsp& rsp);

private:
    bool getFileInfo(const std::string& ip, uint16_t port, GetFileData& fileinfo);
    bool getFileFromOther(const std::string& ip, uint16_t port, const std::string& filepath, uint64_t filesize, bool force_overwrite = false);
    RpcServerConnection* conn_mgr_;
    boost::shared_ptr<FileSyncServer> transfer_rpcserver_;
    boost::mutex mutex_;
    boost::condition_variable cond_;
    std::map<std::string, bool> wait_finish_notify_;
    boost::mutex status_report_mutex_;
    bool  reporting_;
    boost::condition_variable status_report_cond_;
    std::vector<ReportStatusRspData>  status_rsp_list_;
    std::set<std::string>  ignore_list_;
};

}

#endif