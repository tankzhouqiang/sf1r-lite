#ifndef INDEX_BUNDLE_TASK_SERVICE_H
#define INDEX_BUNDLE_TASK_SERVICE_H

#include "IndexBundleConfiguration.h"

#include <node-manager/sharding/ShardingStrategy.h>
#include <aggregator-manager/IndexAggregator.h>
#include <directory-manager/DirectoryRotator.h>
#include <common/Status.h>
#include <common/ScdParser.h>

#include <util/driver/Value.h>
#include <util/osgi/IService.h>

#include <boost/shared_ptr.hpp>

namespace sf1r
{
using izenelib::ir::idmanager::IDManager;

class IndexWorker;
class DocumentManager;
class ScdSharder;

class IndexTaskService : public ::izenelib::osgi::IService
{
public:
    IndexTaskService(IndexBundleConfiguration* bundleConfig);

    ~IndexTaskService();


    bool index(unsigned int numdoc, std::string scd_dir, bool disable_sharding);

    bool index(boost::shared_ptr<DocumentManager>& documentManager, int64_t timestamp);
    bool reindex_from_scd(const std::vector<std::string>& scdlist, int64_t timestamp);

    bool optimizeIndex();

    bool createDocument(const ::izenelib::driver::Value& documentValue);

    bool updateDocument(const ::izenelib::driver::Value& documentValue);
    bool updateDocumentInplace(const ::izenelib::driver::Value& request);

    bool destroyDocument(const ::izenelib::driver::Value& documentValue);

    bool getIndexStatus(Status& status);

    bool isAutoRebuild();

    std::string getScdDir(bool rebuild) const;
    izenelib::util::UString::EncodingType getEncode() const;

    CollectionPath&  getCollectionPath() const;

    boost::shared_ptr<DocumentManager> getDocumentManager() const;

    void flush();
    bool isNeedSharding();
    bool HookDistributeRequestForIndex();
    const std::vector<shardid_t>& getShardidListForSearch();
    bool addNewShardingNodes(const std::vector<shardid_t>& new_sharding_nodes);
    boost::shared_ptr<ShardingStrategy> getShardingStrategy()
    {
        return sharding_strategy_;
    }
    bool generateMigrateSCD(const std::map<shardid_t, std::vector<vnodeid_t> >& scd_list,
        std::map<shardid_t, std::string>& generated_insert_scds,
        std::map<shardid_t, std::string>& generated_del_scds);

private:
    bool SendRequestToSharding(uint32_t shardid);
    bool distributedIndex_(unsigned int numdoc, std::string scd_dir);
    bool distributedIndexImpl_(
        unsigned int numdoc,
        const std::string& collectionName,
        const std::string& masterScdPath);

    bool createScdSharder(
        boost::shared_ptr<ScdSharder>& scdSharder);

    void updateShardingConfig(const std::vector<shardid_t>& new_sharding_nodes);
    bool indexShardingNodes(const std::map<shardid_t, std::vector<std::string> >& generated_migrate_scds);

private:
    std::string service_;
    IndexBundleConfiguration* bundleConfig_;

    boost::shared_ptr<IndexAggregator> indexAggregator_;
    boost::shared_ptr<IndexWorker> indexWorker_;
    boost::shared_ptr<ShardingStrategy> sharding_strategy_;
    boost::shared_ptr<ScdSharder> scdSharder_;
    std::string sharding_map_dir_;

    friend class IndexWorkerController;
    friend class CollectionTaskScheduler;
    friend class IndexBundleActivator;
    friend class ProductBundleActivator;
};

}

#endif
