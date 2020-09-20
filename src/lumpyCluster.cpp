#include <cmath>
#include <vector>

#include "lumpyCluster.h"
#include "treemer.h"

LumpyCluster::Base::Base(const Rcpp::NumericMatrix &metricMatrix):
    m_metricMatrix(metricMatrix) {}

std::vector< std::vector<int> > LumpyCluster::Base::finalClusters() const {
    std::vector< std::vector<int> > res;
    res.reserve(m_merged.size());
    for (
            lumpingTips::const_iterator it = m_merged.begin();
            it != m_merged.end(); ++it
    ) {
        std::vector<int> tipNodes;
        tipNodes.reserve(it->size());
        for (
                Treemer::tips::const_iterator tips_itr = it->begin();
                tips_itr != it->end(); ++tips_itr
        ) {
            tipNodes.push_back((**tips_itr).getTip());
        }
        res.push_back(tipNodes);
    }
    return res;
}

void LumpyCluster::Base::mergeClusters(
        const Treemer::clusters &clusters,
        const int zValue
) {
    // The fist raw cluster
    Treemer::clusters::const_iterator it = clusters.begin();
    m_merged.push_back(it->second);
    // There is no need to merge when there is only one cluster
    if (clusters.size() == 1) {
        return;
    }
    // The reformatted raw clusters input
    lumpingTips rawClusters;
    // Initiate the merged clusters with the first raw clusters so the size of
    // the reformatted raw cluster will be one size less
    rawClusters.reserve(clusters.size()-1);
    // Collect all tips for later calculating the average paired metric
    Treemer::tips allTips = it->second;
    // Calculate the threshold value and reformat the data structure from map to
    // vector
    for (++it; it != clusters.end(); ++it) {
        // Reformat
        rawClusters.push_back(it->second);
        // Collect tips
        allTips.insert(
            allTips.begin(),
            it->second.begin(),
            it->second.end()
        );
    }
    int count = 0;
    float metricSum = 0.0;
    float squareSum = 0.0;
    for (
            Treemer::tips::iterator it1 = allTips.begin();
            it1 != allTips.end()-1; ++it1
    ) {
        for (
                Treemer::tips::iterator it2 = it1 + 1;
                it2 != allTips.end(); ++it2
        ) {
            count++;
            float pairMetric = m_metricMatrix(
                (**it1).getTip(),
                (**it2).getTip()
            );
            metricSum += pairMetric;
            squareSum += pairMetric*pairMetric;
        }
    }
    float average = metricSum/count;
    float variance = (count*squareSum-metricSum*metricSum)/(count*count);
    // The distance and similarity metric will have different offsetting
    setThreshold(average, std::sqrt(variance), zValue);
    // Iterate the raw clusters from treemerBySite as the candidate for merging
    // or adding
    for (
            lumpingTips::const_iterator candidate = rawClusters.begin();
            candidate != rawClusters.end(); ++candidate
    ) {
        // Iterate the merged clusters to find the best one for the current raw
        // candidate cluster. But the merging metric has to pass the threshold.
        // Assume the first one is the best to be merged with the candidate.
        lumpingTips::iterator toMerge = m_merged.begin();
        float bestMetric = clusterCompare(*candidate, *toMerge);
        for (
                lumpingTips::iterator it = m_merged.begin()+1;
                it != m_merged.end(); ++it
        ) {
            float metric = clusterCompare(*candidate, *it);
            // Replace the best if the current is better to be merged with the
            // candidate
            if (betterMetric(metric, bestMetric)) {
                toMerge = it;
                bestMetric = metric;
            }
        }
        // The best metric has to pass the threshold or a new cluster is formed
        if (qualifiedMetric(bestMetric)) {
            // The raw candidate cluster is merged
            toMerge->insert(
                    toMerge->end(),
                    candidate->begin(),
                    candidate->end()
            );
        } else {
            // A cluster is formed
            m_merged.push_back(*candidate);
        }
    }
}

float LumpyCluster::Base::clusterCompare(
        const Treemer::tips &query,
        const Treemer::tips &subject
) {
    int count = 0;
    float metricSum = 0.0;
    // The average metric between tip pairs from each two clusters to
    // approximate the metric between the two clusters
    for (
            Treemer::tips::const_iterator it1 = query.begin();
            it1 != query.end(); ++it1
    ) {
        for (
                Treemer::tips::const_iterator it2 = subject.begin();
                it2 != subject.end(); ++it2
        ) {
            metricSum += m_metricMatrix(
                (**it1).getTip(),
                (**it2).getTip()
            );
            count++;
        }
    }
    return metricSum/count;
}

LumpyCluster::BySimMatrix::BySimMatrix(
    const Rcpp::NumericMatrix &simMatrix,
    const Treemer::clusters &clusters,
    const int zValue
):
    Base(simMatrix) {
    mergeClusters(clusters, zValue);
}

void LumpyCluster::BySimMatrix::setThreshold(
        const float average,
        const float stdev,
        const int zValue
) {
    // When using similarity, the number goes up to indicate closer
    // relationship.
    m_metricThreshold = average+stdev*zValue;
}

bool LumpyCluster::BySimMatrix::betterMetric(
        const float query,
        const float subject
) const {
    return query > subject;
}

bool LumpyCluster::BySimMatrix::qualifiedMetric(const float metric) const {
    return metric > m_metricThreshold;
}

LumpyCluster::ByDistMatrix::ByDistMatrix(
    const Rcpp::NumericMatrix &simMatrix,
    const Treemer::clusters &clusters,
    const int zValue
):
    Base(simMatrix) {
    mergeClusters(clusters, zValue);
}

void LumpyCluster::ByDistMatrix::setThreshold(
        const float average,
        const float stdev,
        const int zValue
) {
    // When using distance, the number goes down to indicate closer
    // relationship.
    m_metricThreshold = average-stdev*zValue;
}

bool LumpyCluster::ByDistMatrix::betterMetric(
        const float query,
        const float subject
) const {
    return query < subject;
}

bool LumpyCluster::ByDistMatrix::qualifiedMetric(const float metric) const {
    return metric < m_metricThreshold;
}

template<class T>
LumpyCluster::tipNodes LumpyCluster::terminalTips(
        const Rcpp::ListOf<Rcpp::IntegerVector> &tipPaths,
        const Rcpp::ListOf<Rcpp::CharacterVector> &alignedSeqs,
        const Rcpp::NumericMatrix &simMatrix,
        const Rcpp::IntegerVector &siteIndices,
        const int minSNPnum,
        const int zValue
) {
    Treemer::tips tips;
    Treemer::clusters initClusters;
    // Iterate tipPaths and alignedSeqs to construct a list of TipSeqLinkers
    for (int i = 0; i < tipPaths.size(); i++) {
        Treemer::TipSeqLinker *tip = new Treemer::TipSeqLinker(
            alignedSeqs[i], tipPaths[i]
        );
        tips.push_back(tip);
        // The initial clustering is each tip as a cluster
        initClusters[tip->getTip()].push_back(tip);
        // The root of each tipPath should be the same. The sequences should be
        // of the same length
        if (tips[i]->getRoot() != tips[0]->getRoot()) {
            throw std::invalid_argument("Root in tree paths not equal");
        } else if (tips[i]->getSeqLen() != tips[0]->getSeqLen()) {
            throw std::invalid_argument("Sequence length not equal");
        }
    }
    tipNodes res;
    for (
            Rcpp::IntegerVector::const_iterator it = siteIndices.begin();
            it != siteIndices.end(); it++
    ) {
        Treemer::BySite matched(tips, initClusters, *it);
        clsByAA siteClusters = matched.siteClusters();
        for (
                clsByAA::iterator clusters_itr = siteClusters.begin();
                clusters_itr != siteClusters.end(); ++clusters_itr
        ) {
            T merger(simMatrix, clusters_itr->second, zValue);
            tipNodes mergedCls = merger.finalClusters();
            for (
                    tipNodes::iterator cls_itr = mergedCls.begin();
                    cls_itr != mergedCls.end(); ++cls_itr
            ) {
                if (cls_itr->size() >= minSNPnum) {
                    res.push_back(*cls_itr);
                }
            }
        }
    }
    // Manually free the memory allocated by new
    for (Treemer::tips::iterator it = tips.begin(); it != tips.end(); ++it) {
        delete *it;
    }
    return res;
}

template LumpyCluster::tipNodes
    LumpyCluster::terminalTips<LumpyCluster::BySimMatrix>(
        const Rcpp::ListOf<Rcpp::IntegerVector> &tipPaths,
        const Rcpp::ListOf<Rcpp::CharacterVector> &alignedSeqs,
        const Rcpp::NumericMatrix &simMatrix,
        const Rcpp::IntegerVector &siteIndices,
        const int minSNPnum,
        const int zValue
);

template LumpyCluster::tipNodes
    LumpyCluster::terminalTips<LumpyCluster::ByDistMatrix>(
        const Rcpp::ListOf<Rcpp::IntegerVector> &tipPaths,
        const Rcpp::ListOf<Rcpp::CharacterVector> &alignedSeqs,
        const Rcpp::NumericMatrix &simMatrix,
        const Rcpp::IntegerVector &siteIndices,
        const int minSNPnum,
        const int zValue
);
