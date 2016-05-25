//
//  individual_caller.hpp
//  Octopus
//
//  Created by Daniel Cooke on 01/04/2016.
//  Copyright © 2016 Oxford University. All rights reserved.
//

#ifndef individual_caller_hpp
#define individual_caller_hpp

#include <vector>
#include <string>
#include <memory>

#include <boost/optional.hpp>

#include "common.hpp"
#include "variant_caller.hpp"
#include "individual_genotype_model.hpp"
#include "variant_call.hpp"

class GenomicRegion;
class ReadPipe;
class Variant;
class HaplotypeLikelihoodCache;

namespace Octopus
{
class IndividualVariantCaller : public VariantCaller
{
public:
    struct CallerParameters
    {
        CallerParameters() = default;
        explicit CallerParameters(double min_variant_posterior, double min_refcall_posterior,
                                  unsigned ploidy);
        ~CallerParameters() = default;
        
        double min_variant_posterior;
        double min_refcall_posterior;
        unsigned ploidy;
    };
    
    IndividualVariantCaller() = delete;
    
    explicit IndividualVariantCaller(const ReferenceGenome& reference,
                                     ReadPipe& read_pipe,
                                     CandidateVariantGenerator&& candidate_generator,
                                     VariantCaller::CallerParameters general_parameters,
                                     CallerParameters specific_parameters);
    
    ~IndividualVariantCaller() = default;
    
    IndividualVariantCaller(const IndividualVariantCaller&)            = delete;
    IndividualVariantCaller& operator=(const IndividualVariantCaller&) = delete;
    IndividualVariantCaller(IndividualVariantCaller&&)                 = delete;
    IndividualVariantCaller& operator=(IndividualVariantCaller&&)      = delete;
    
private:
    class Latents : public CallerLatents
    {
    public:
        using ModelInferences = GenotypeModel::Individual::InferredLatents;
        
        using CallerLatents::HaplotypeProbabilityMap;
        using CallerLatents::GenotypeProbabilityMap;
        
        friend IndividualVariantCaller;
        
        explicit Latents(const SampleIdType& sample,
                         const std::vector<Haplotype>&,
                         std::vector<Genotype<Haplotype>>&& genotypes,
                         ModelInferences&&);
        explicit Latents(const SampleIdType& sample,
                         const std::vector<Haplotype>&,
                         std::vector<Genotype<Haplotype>>&& genotypes,
                         ModelInferences&&, ModelInferences&&);
        
        std::shared_ptr<HaplotypeProbabilityMap> get_haplotype_posteriors() const noexcept override;
        std::shared_ptr<GenotypeProbabilityMap> get_genotype_posteriors() const noexcept override;
        
    private:
        std::shared_ptr<GenotypeProbabilityMap> genotype_posteriors_;
        std::shared_ptr<HaplotypeProbabilityMap> haplotype_posteriors_;
        
        boost::optional<ModelInferences> dummy_latents_;
        
        double model_log_evidence_;
        
        HaplotypeProbabilityMap
        calculate_haplotype_posteriors(const std::vector<Haplotype>& haplotypes);
    };
    
    double min_variant_posterior_;
    double min_refcall_posterior_;
    unsigned ploidy_;
    
    std::unique_ptr<CallerLatents>
    infer_latents(const std::vector<Haplotype>& haplotypes,
                  const HaplotypeLikelihoodCache& haplotype_likelihoods) const override;
    
    std::vector<std::unique_ptr<VariantCall>>
    call_variants(const std::vector<Variant>& candidates, CallerLatents& latents) const override;
    
    std::vector<std::unique_ptr<VariantCall>>
    call_variants(const std::vector<Variant>& candidates, const Latents& latents) const;
    
    std::vector<std::unique_ptr<ReferenceCall>>
    call_reference(const std::vector<Allele>& alleles, CallerLatents& latents,
                   const ReadMap& reads) const override;
    
    const SampleIdType& sample() const noexcept;
};
} // namespace Octopus

#endif /* individual_caller_hpp */
