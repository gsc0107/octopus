// Copyright (c) 2015-2018 Daniel Cooke
// Use of this source code is governed by the MIT license that can be found in the LICENSE file.

#ifndef x10_indel_error_model_hpp
#define x10_indel_error_model_hpp

#include "core/models/error/indel_error_model.hpp"

namespace octopus {

class Haplotype;

class X10IndelErrorModel : public IndelErrorModel
{
public:
    using IndelErrorModel::PenaltyType;
    using IndelErrorModel::PenaltyVector;
    
    X10IndelErrorModel() = default;
    
    X10IndelErrorModel(const X10IndelErrorModel&)            = default;
    X10IndelErrorModel& operator=(const X10IndelErrorModel&) = default;
    X10IndelErrorModel(X10IndelErrorModel&&)                 = default;
    X10IndelErrorModel& operator=(X10IndelErrorModel&&)      = default;

private:
    static constexpr std::array<PenaltyType, 50> homopolymerErrors_ =
    {{
     60,60,49,44,40,35,28,24,21,19,18,16,15,15,14,13,12,11,11,10,
     9,9,8,8,7,7,7,6,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,2,2,1,1,1,1,1
     }};
    static constexpr std::array<PenaltyType, 50> diNucleotideTandemRepeatErrors_ =
    {{
     60,59,49,45,41,36,30,26,22,21,20,19,18,17,15,13,12,11,11,10,
     9,9,8,8,7,7,7,6,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,2,2,1,1,1,1,1
     }};
    static constexpr std::array<PenaltyType, 50> triNucleotideTandemRepeatErrors_ =
    {{
     60,59,49,47,45,43,39,37,34,30,27,24,21,18,16,14,13,12,12,11,
     10,9,8,8,7,7,7,6,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,2,2,1,1,1,1,1
     }};
    
    static constexpr std::array<PenaltyType, 50> polyNucleotideTandemRepeatErrors_ =
    {{
     60,60,50,44,44,44,44,44,22,19,18,16,16,15,14,13,12,11,11,10,
     9,9,8,8,7,7,7,6,6,6,5,5,5,4,4,4,3,3,3,3,2,2,2,2,2,1,1,1,1,1
     }};
    
    static constexpr PenaltyType defaultGapExtension_ = 3;
    
    virtual std::unique_ptr<IndelErrorModel> do_clone() const override;
    virtual PenaltyType do_evaluate(const Haplotype& haplotype, PenaltyVector& gap_open_penalties) const override;
};
    
} // namespace octopus

#endif
