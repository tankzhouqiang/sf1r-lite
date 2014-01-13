/**
 * @file KNlpProductTokenizer.h
 * @brief KNlp tokenizer.
 */

#ifndef KNLP_PRODUCT_TOKENIZER_H
#define KNLP_PRODUCT_TOKENIZER_H

#include "ProductTokenizer.h"
#include "MatcherProductTokenizer.h"

namespace sf1r
{

class KNlpProductTokenizer : public ProductTokenizer
{
public:
    virtual void tokenize(ProductTokenParam& param);

    virtual double sumQueryScore(const std::string& query);

    virtual void setProductMatcher(b5m::ProductMatcher* matcher)
    {
        matcherTokenizer_.setProductMatcher(matcher);
    }

private:
    double tokenizeImpl_(ProductTokenParam& param);

    void getRankBoundary_(ProductTokenParam& param);

    void getRefinedResult_(ProductTokenParam& param);

private:
    MatcherProductTokenizer matcherTokenizer_;
};

}

#endif // KNLP_PRODUCT_TOKENIZER_H
