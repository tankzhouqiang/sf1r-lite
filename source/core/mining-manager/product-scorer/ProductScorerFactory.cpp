#include "ProductScorerFactory.h"
#include "ProductScoreSum.h"
#include "CustomScorer.h"
#include "CategoryScorer.h"
#include "PopularityScorer.h"
#include "NumericPropertyScorer.h"
#include "../MiningManager.h"
#include "../custom-rank-manager/CustomRankManager.h"
#include "../group-label-logger/GroupLabelLogger.h"
#include "../group-manager/PropSharedLockSet.h"
#include <configuration-manager/ProductRankingConfig.h>
#include <search-manager/SearchManager.h>
#include <memory> // auto_ptr

using namespace sf1r;

namespace
{
/**
 * in order to make the category score less than 10 (the minimum custom
 * score), we would select at most 9 top labels.
 */
const score_t kTopLabelLimit = 9;
}

ProductScorerFactory::ProductScorerFactory(
    const ProductRankingConfig& config,
    MiningManager& miningManager)
    : config_(config)
    , customRankManager_(miningManager.GetCustomRankManager())
    , categoryClickLogger_(NULL)
    , categoryValueTable_(NULL)
    , searchManager_(miningManager.GetSearchManager())
{
    const ProductScoreConfig& categoryScoreConfig =
        config.scores[CATEGORY_SCORE];
    const std::string& categoryProp = categoryScoreConfig.propName;
    categoryClickLogger_ = miningManager.GetGroupLabelLogger(categoryProp);
    categoryValueTable_ = miningManager.GetPropValueTable(categoryProp);
}

ProductScorer* ProductScorerFactory::createScorer(
    const std::string& query,
    faceted::PropSharedLockSet& propSharedLockSet,
    ProductScorer* relevanceScorer)
{
    std::auto_ptr<ProductScoreSum> scoreSum(new ProductScoreSum);

    bool isany = false;
    for (int i = 0; i < PRODUCT_SCORE_NUM; ++i)
    {
        ProductScorer* scorer = createScorerImpl_(config_.scores[i],
                                                  query,
                                                  propSharedLockSet,
                                                  relevanceScorer);
        if (scorer)
        {
            scoreSum->addScorer(scorer);
            isany = true;
        }
    }

    if(!isany)
        return NULL;

    return scoreSum.release();
}

ProductScorer* ProductScorerFactory::createScorerImpl_(
    const ProductScoreConfig& scoreConfig,
    const std::string& query,
    faceted::PropSharedLockSet& propSharedLockSet,
    ProductScorer* relevanceScorer)
{
    if (scoreConfig.weight == 0)
        return NULL;

    switch(scoreConfig.type)
    {
    case CUSTOM_SCORE:
        return createCustomScorer_(scoreConfig, query);

    case CATEGORY_SCORE:
        return createCategoryScorer_(scoreConfig, query, propSharedLockSet);

    case RELEVANCE_SCORE:
        return createRelevanceScorer_(scoreConfig, relevanceScorer);

    case POPULARITY_SCORE:
        return createPopularityScorer_(scoreConfig);

    default:
        return NULL;
    }
}

ProductScorer* ProductScorerFactory::createCustomScorer_(
    const ProductScoreConfig& scoreConfig,
    const std::string& query)
{
    if (customRankManager_ == NULL)
        return NULL;

    CustomRankDocId customDocId;
    bool result = customRankManager_->getCustomValue(query, customDocId);

    if (result && !customDocId.topIds.empty())
        return new CustomScorer(scoreConfig, customDocId.topIds);

    return NULL;
}

ProductScorer* ProductScorerFactory::createCategoryScorer_(
    const ProductScoreConfig& scoreConfig,
    const std::string& query,
    faceted::PropSharedLockSet& propSharedLockSet)
{
    if (categoryClickLogger_ == NULL ||
        categoryValueTable_ == NULL)
        return NULL;

    std::vector<faceted::PropValueTable::pvid_t> topLabels;
    std::vector<int> topFreqs;
    bool result = categoryClickLogger_->getFreqLabel(query, kTopLabelLimit,
                                                     topLabels, topFreqs);

    if (result && !topLabels.empty())
    {
        propSharedLockSet.insertSharedLock(categoryValueTable_);
        return new CategoryScorer(scoreConfig, *categoryValueTable_, topLabels);
    }

    return NULL;
}

ProductScorer* ProductScorerFactory::createRelevanceScorer_(
    const ProductScoreConfig& scoreConfig,
    ProductScorer* relevanceScorer)
{
    if (!relevanceScorer)
        return NULL;

    relevanceScorer->setWeight(scoreConfig.weight);
    return relevanceScorer;
}

ProductScorer* ProductScorerFactory::createPopularityScorer_(
    const ProductScoreConfig& scoreConfig)
{
    std::auto_ptr<PopularityScorer> popularScorer(
        new PopularityScorer(scoreConfig));

    for (std::size_t i = 0; i < scoreConfig.factors.size(); ++i)
    {
        const ProductScoreConfig& factorConfig = scoreConfig.factors[i];
        ProductScorer* scorer = createNumericPropertyScorer_(factorConfig);

        if (scorer)
        {
            popularScorer->addScorer(scorer);
        }
    }

    return popularScorer.release();
}

ProductScorer* ProductScorerFactory::createNumericPropertyScorer_(
    const ProductScoreConfig& scoreConfig)
{
    const std::string& propName = scoreConfig.propName;
    if (propName.empty() || scoreConfig.weight == 0)
        return NULL;

    if (!searchManager_)
    {
        LOG(WARNING) << "failed to get SearchManager";
        return NULL;
    }

    boost::shared_ptr<NumericPropertyTableBase>& numericTable =
        searchManager_->createPropertyTable(propName);

    if (!numericTable)
    {
        LOG(WARNING) << "failed to create NumericPropertyTableBase "
                     << "for property [" << propName << "]";
        return NULL;
    }

    return new NumericPropertyScorer(scoreConfig, numericTable);
}