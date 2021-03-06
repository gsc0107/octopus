// Copyright (c) 2015-2018 Daniel Cooke
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.

#include "germline_likelihood_model.hpp"

#include <vector>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <array>
#include <limits>
#include <cassert>

#include "utils/maths.hpp"

namespace octopus { namespace model {

GermlineLikelihoodModel::GermlineLikelihoodModel(const HaplotypeLikelihoodCache& likelihoods)
: likelihoods_ {likelihoods}
{}

// ln p(read | genotype)  = ln sum {haplotype in genotype} p(read | haplotype) - ln ploidy
// ln p(reads | genotype) = sum {read in reads} ln p(read | genotype)
double GermlineLikelihoodModel::evaluate(const Genotype<Haplotype>& genotype) const
{
    assert(likelihoods_.is_primed());
    // These cases are just for optimisation
    switch (genotype.ploidy()) {
        case 0:
            return 0.0;
        case 1:
            return evaluate_haploid(genotype);
        case 2:
            return evaluate_diploid(genotype);
        case 3:
            return evaluate_triploid(genotype);
        case 4:
            return evaluate_polyploid(genotype);
            //return log_likelihood_tetraploid(sample, genotype);
        default:
            return evaluate_polyploid(genotype);
    }
}

// private methods

namespace {

template <typename T = double>
static constexpr auto ln(const unsigned n)
{
    constexpr std::array<T, 11> lnLookup {
        std::numeric_limits<T>::infinity(),
        0.0,
        0.693147180559945309417232121458176568075500134360255254120,
        1.098612288668109691395245236922525704647490557822749451734,
        1.386294361119890618834464242916353136151000268720510508241,
        1.609437912434100374600759333226187639525601354268517721912,
        1.791759469228055000812477358380702272722990692183004705855,
        1.945910149055313305105352743443179729637084729581861188459,
        2.079441541679835928251696364374529704226500403080765762362,
        2.197224577336219382790490473845051409294981115645498903469,
        2.302585092994045684017991454684364207601101488628772976033
    };
    return lnLookup[n];
}
    
} // namespace

double GermlineLikelihoodModel::evaluate_haploid(const Genotype<Haplotype>& genotype) const
{
    const auto& log_likelihoods = likelihoods_[genotype[0]];
    return std::accumulate(std::cbegin(log_likelihoods), std::cend(log_likelihoods), 0.0);
}

double GermlineLikelihoodModel::evaluate_diploid(const Genotype<Haplotype>& genotype) const
{
    const auto& log_likelihoods1 = likelihoods_[genotype[0]];
    if (genotype.is_homozygous()) {
        return std::accumulate(std::cbegin(log_likelihoods1), std::cend(log_likelihoods1), 0.0);
    }
    const auto& log_likelihoods2 = likelihoods_[genotype[1]];
    return std::inner_product(std::cbegin(log_likelihoods1), std::cend(log_likelihoods1),
                              std::cbegin(log_likelihoods2), 0.0, std::plus<> {},
                              [] (const auto a, const auto b) -> double {
                                  return maths::log_sum_exp(a, b) - ln<>(2);
                              });
}

double GermlineLikelihoodModel::evaluate_triploid(const Genotype<Haplotype>& genotype) const
{
    using std::cbegin; using std::cend;
    
    const auto& log_likelihoods1 = likelihoods_[genotype[0]];
    if (genotype.is_homozygous()) {
        return std::accumulate(cbegin(log_likelihoods1), cend(log_likelihoods1), 0.0);
    }
    if (genotype.zygosity() == 3) {
        const auto& log_likelihoods2 = likelihoods_[genotype[1]];
        const auto& log_likelihoods3 = likelihoods_[genotype[2]];
        return maths::inner_product(cbegin(log_likelihoods1), cend(log_likelihoods1),
                                    cbegin(log_likelihoods2), cbegin(log_likelihoods3),
                                    0.0, std::plus<> {},
                                    [] (const auto a, const auto b, const auto c) -> double {
                                        return maths::log_sum_exp(a, b, c) - ln<>(3);
                                    });
    }
    if (genotype[0] != genotype[1]) {
        const auto& log_likelihoods2 = likelihoods_[genotype[1]];
        return std::inner_product(cbegin(log_likelihoods1), cend(log_likelihoods1),
                                  cbegin(log_likelihoods2), 0.0, std::plus<> {},
                                  [] (const auto a, const auto b) -> double {
                                      return maths::log_sum_exp(a, ln<>(2) + b) - ln<>(3);
                                  });
    }
    const auto& log_likelihoods3 = likelihoods_[genotype[2]];
    return std::inner_product(cbegin(log_likelihoods1), cend(log_likelihoods1),
                              cbegin(log_likelihoods3), 0.0, std::plus<> {},
                              [] (const auto a, const auto b) -> double {
                                  return maths::log_sum_exp(ln<>(2) + a, b) - ln<>(3);
                              });
}

double GermlineLikelihoodModel::evaluate_tetraploid(const Genotype<Haplotype>& genotype) const
{
    const auto z = genotype.zygosity();
    const auto& log_likelihoods1 = likelihoods_[genotype[0]];
    if (z == 1) {
        return std::accumulate(std::cbegin(log_likelihoods1), std::cend(log_likelihoods1), 0.0);
    }
    if (z == 4) {
        const auto& log_likelihoods2 = likelihoods_[genotype[1]];
        const auto& log_likelihoods3 = likelihoods_[genotype[2]];
        const auto& log_likelihoods4 = likelihoods_[genotype[3]];
        return maths::inner_product(std::cbegin(log_likelihoods1), std::cend(log_likelihoods1),
                                    std::cbegin(log_likelihoods2), std::cbegin(log_likelihoods3),
                                    std::cbegin(log_likelihoods4), 0.0, std::plus<> {},
                                    [] (const auto a, const auto b, const auto c, const auto d) -> double {
                                        return maths::log_sum_exp({a, b, c, d}) - ln<>(4);
                                    });
    }
    
    // TODO
    
    return 0;
}

double GermlineLikelihoodModel::evaluate_polyploid(const Genotype<Haplotype>& genotype) const
{
    const auto ploidy = genotype.ploidy();
    const auto z = genotype.zygosity();
    const auto& log_likelihoods1 = likelihoods_[genotype[0]];
    
    if (z == 1) {
        return std::accumulate(std::cbegin(log_likelihoods1), std::cend(log_likelihoods1), 0.0);
    }
    if (z == 2) {
        const double lnpm1 {std::log(ploidy - 1)};
        const auto unique_haplotypes = genotype.copy_unique_ref();
        const auto& log_likelihoods2 = likelihoods_[unique_haplotypes.back()];
        
        if (genotype.count(unique_haplotypes.front()) == 1) {
            return std::inner_product(std::cbegin(log_likelihoods1), std::cend(log_likelihoods1),
                                      std::cbegin(log_likelihoods2), 0.0, std::plus<> {},
                                      [ploidy, lnpm1] (const auto a, const auto b) -> double {
                                          return maths::log_sum_exp(a, lnpm1 + b) - ln<>(ploidy);
                                      });
        }
        return std::inner_product(std::cbegin(log_likelihoods1), std::cend(log_likelihoods1),
                                  std::cbegin(log_likelihoods2), 0.0, std::plus<> {},
                                  [ploidy, lnpm1] (const auto a, const auto b) -> double {
                                      return maths::log_sum_exp(lnpm1 + a, b) - ln<>(ploidy);
                                  });
    }
    
    std::vector<HaplotypeLikelihoodCache::LikelihoodVectorRef> ln_likelihoods {};
    ln_likelihoods.reserve(ploidy);
    
    std::transform(std::cbegin(genotype), std::cend(genotype), std::back_inserter(ln_likelihoods),
                   [this] (const auto& haplotype)
                        -> const HaplotypeLikelihoodCache::LikelihoodVector& {
                       return likelihoods_[haplotype];
                   });
    
    std::vector<double> tmp(ploidy);
    double result {0};
    const auto num_likelihoods = ln_likelihoods.front().get().size();
    
    for (std::size_t i {0}; i < num_likelihoods; ++i) {
        std::transform(std::cbegin(ln_likelihoods), std::cend(ln_likelihoods), std::begin(tmp),
                       [i] (const auto& likelihoods) {
                           return likelihoods.get()[i];
                       });
        result += maths::log_sum_exp(tmp) - ln<>(ploidy);
    }
    
    return result;
}

} // namespace model
} // namespace octopus
