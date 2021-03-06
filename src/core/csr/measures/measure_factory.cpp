// Copyright (c) 2015-2018 Daniel Cooke
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.

#include "measure_factory.hpp"

#include <unordered_map>
#include <functional>

#include "measures_fwd.hpp"
#include "exceptions/user_error.hpp"
#include "utils/map_utils.hpp"

namespace octopus { namespace csr {

using MeasureMakerMap = std::unordered_map<std::string, std::function<MeasureWrapper()>>;

void init(MeasureMakerMap& measure_makers)
{
    measure_makers[name<AlleleFrequency>()]             = [] () { return make_wrapped_measure<AlleleFrequency>(); };
    measure_makers[name<Depth>()]                       = [] () { return make_wrapped_measure<Depth>(); };
    measure_makers[name<MappingQualityDivergence>()]    = [] () { return make_wrapped_measure<MappingQualityDivergence>(); };
    measure_makers[name<MappingQualityZeroCount>()]     = [] () { return make_wrapped_measure<MappingQualityZeroCount>(); };
    measure_makers[name<MeanMappingQuality>()]          = [] () { return make_wrapped_measure<MeanMappingQuality>(); };
    measure_makers[name<ModelPosterior>()]              = [] () { return make_wrapped_measure<ModelPosterior>(); };
    measure_makers[name<Quality>()]                     = [] () { return make_wrapped_measure<Quality>(); };
    measure_makers[name<QualityByDepth>()]              = [] () { return make_wrapped_measure<QualityByDepth>(); };
    measure_makers[name<GenotypeQuality>()]             = [] () { return make_wrapped_measure<GenotypeQuality>(); };
    measure_makers[name<StrandBias>()]                  = [] () { return make_wrapped_measure<StrandBias>(); };
    measure_makers[name<GCContent>()]                   = [] () { return make_wrapped_measure<GCContent>(); };
    measure_makers[name<FilteredReadFraction>()]        = [] () { return make_wrapped_measure<FilteredReadFraction>(); };
    measure_makers[name<ClippedReadFraction>()]         = [] () { return make_wrapped_measure<ClippedReadFraction>(); };
    measure_makers[name<IsDenovo>()]                    = [] () { return make_wrapped_measure<IsDenovo>(); };
    measure_makers[name<IsSomatic>()]                   = [] () { return make_wrapped_measure<IsSomatic>(); };
    measure_makers[name<AmbiguousReadFraction>()]       = [] () { return make_wrapped_measure<AmbiguousReadFraction>(); };
    measure_makers[name<MedianBaseQuality>()]           = [] () { return make_wrapped_measure<MedianBaseQuality>(); };
    measure_makers[name<MismatchCount>()]               = [] () { return make_wrapped_measure<MismatchCount>(); };
    measure_makers[name<MismatchFraction>()]            = [] () { return make_wrapped_measure<MismatchFraction>(); };
    measure_makers[name<IsRefcall>()]                   = [] () { return make_wrapped_measure<IsRefcall>(); };
    measure_makers[name<SomaticContamination>()]        = [] () { return make_wrapped_measure<SomaticContamination>(); };
    measure_makers[name<DeNovoContamination>()]         = [] () { return make_wrapped_measure<DeNovoContamination>(); };
    measure_makers[name<ReadPositionBias>()]            = [] () { return make_wrapped_measure<ReadPositionBias>(); };
    measure_makers[name<AltAlleleCount>()]              = [] () { return make_wrapped_measure<AltAlleleCount>(); };
    measure_makers[name<OverlapsTandemRepeat>()]        = [] () { return make_wrapped_measure<OverlapsTandemRepeat>(); };
    measure_makers[name<STRLength>()]                   = [] () { return make_wrapped_measure<STRLength>(); };
    measure_makers[name<STRPeriod>()]                   = [] () { return make_wrapped_measure<STRPeriod>(); };
    measure_makers[name<PosteriorProbability>()]        = [] () { return make_wrapped_measure<PosteriorProbability>(); };
    measure_makers[name<PosteriorProbabilityByDepth>()] = [] () { return make_wrapped_measure<PosteriorProbabilityByDepth>(); };
    measure_makers[name<ClassificationConfidence>()]    = [] () { return make_wrapped_measure<ClassificationConfidence>(); };
    measure_makers[name<SomaticHaplotypeCount>()]       = [] () { return make_wrapped_measure<SomaticHaplotypeCount>(); };
    measure_makers[name<MedianSomaticMappingQuality>()] = [] () { return make_wrapped_measure<MedianSomaticMappingQuality>(); };
}

class UnknownMeasure : public UserError
{
    std::string name_;
    std::string do_where() const override { return "make_measure"; }
    std::string do_why() const override
    {
        return name_ + std::string {" is not a valid measure"};
    }
    std::string do_help() const override
    {
        return "See the documentation for valid measures";
    }
public:
    UnknownMeasure(std::string name) : name_ {std::move(name)} {}
};

MeasureWrapper make_measure(const std::string& name)
{
    static MeasureMakerMap measure_makers {};
    if (measure_makers.empty()) {
        init(measure_makers);
    }
    if (measure_makers.count(name) == 0) {
        throw UnknownMeasure {name};
    }
    return measure_makers.at(name)();
}

std::vector<std::string> get_all_measure_names()
{
    static MeasureMakerMap measure_makers {};
    if (measure_makers.empty()) {
        init(measure_makers);
    }
    return extract_sorted_keys(measure_makers);
}

} // namespace csr
} // namespace octopus
