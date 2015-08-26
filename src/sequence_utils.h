//
//  sequence_utils.h
//  Octopus
//
//  Created by Daniel Cooke on 14/08/2015.
//  Copyright (c) 2015 Oxford University. All rights reserved.
//

#ifndef __Octopus__sequence_utils__
#define __Octopus__sequence_utils__

#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <cstddef>
#include <iterator>
#include <algorithm> // std::count_if
#include <random>

#include <iostream> // TEST

#include "contig_region.h"
#include "tandem.h"

namespace detail
{
    static const constexpr char Unknown_base {'N'};
    static const constexpr std::array<char, 4> Dna_bases {'A', 'C', 'G', 'T'};
    static const constexpr std::array<char, 4> Rna_bases {'A', 'C', 'G', 'U'};
} // end namespace detail

template <typename SequenceType>
bool has_ns(const SequenceType& sequence)
{
    return std::any_of(std::cbegin(sequence), std::cend(sequence), 'N');
}

template <typename SequenceType>
bool is_dna(const SequenceType& sequence)
{
    return sequence.find_first_not_of("ACGTN") == SequenceType::npos;
}

template <typename SequenceType>
bool is_rna(const SequenceType& sequence)
{
    return sequence.find_first_not_of("ACGUN") == SequenceType::npos;
}

template <typename SequenceType>
bool is_dna_rna_ambiguous(const SequenceType& sequence) // i.e. is_dna(sequence) && is_rna(sequence) == true
{
    return sequence.find_first_of("TU") != SequenceType::npos;
}

template <typename SequenceType>
SequenceType transcribe(SequenceType dna_sequence)
{
    std::replace(std::begin(dna_sequence), std::end(dna_sequence), 'T', 'U');
    return dna_sequence;
}

template <typename SequenceType>
SequenceType reverse_transcribe(SequenceType rna_sequence)
{
    std::replace(std::begin(rna_sequence), std::end(rna_sequence), 'U', 'T');
    return rna_sequence;
}

template <typename SequenceType>
bool has_mixed_case(SequenceType& sequence)
{
    return false;
}

template <typename SequenceType>
void capitalise(SequenceType& sequence)
{
    std::transform(std::begin(sequence), std::end(sequence), std::begin(sequence),
                   [] (char base) {
                       switch (base) {
                           case 'a': return 'A';
                           case 'c': return 'C';
                           case 'g': return 'G';
                           case 't': return 'T';
                           case 'u': return 'U';
                           default : return base;
                       }
                   });
}

template <typename SequenceType>
void randomise_ns(SequenceType& sequence)
{
    
}

namespace detail
{
    static const constexpr std::array<char, 128> rc_table {
        4, 4,  4, 4,  4,  4,  4, 4,  4, 4, 4, 4,  4, 4, 4,  4,
        4, 4,  4, 4,  4,  4,  4, 4,  4, 4, 4, 4,  4, 4, 4,  4,
        4, 4,  4, 4,  4,  4,  4, 4,  4, 4, 4, 4,  4, 4, 4,  4,
        4, 4,  4, 4,  4,  4,  4, 4,  4, 4, 4, 4,  4, 4, 4,  4,
        4, 84, 4, 71, 4,  4,  4, 67, 4, 4, 4, 4,  4, 4, 78, 4,
        4, 4,  4, 4,  65, 65, 4, 4,  4, 4, 4, 4,  4, 4, 4,  4,
        4, 84, 4, 71, 4,  4,  4, 67, 4, 4, 4, 4,  4, 4, 4,  4,
        4, 4,  4, 4,  65, 65, 4, 4,  4, 4, 4, 4,  4, 4, 4,  4
    };
} // end namespace detail

inline constexpr char complement(char base)
{
    return detail::rc_table[base];
}

template <typename SequenceType>
SequenceType reverse_complement(SequenceType sequence)
{
    auto f_itr = std::begin(sequence);
    auto r_itr = std::prev(std::end(sequence));
    
    for (; f_itr < r_itr; ++f_itr, --r_itr) {
        char c {complement(*f_itr)};
        *f_itr = complement(*r_itr);
        *r_itr = c;
    }
    
    if (f_itr == r_itr) {
        *f_itr = complement(*f_itr); // complement middle base if sequence is odd length
    }
    
    return sequence;
}

template <typename SequenceType>
bool is_palindromic(const SequenceType& sequence)
{
    if (sequence.empty() || sequence.size() % 2 != 0) return false;
    
    for (auto f_itr = std::cbegin(sequence), r_itr = std::prev(std::cend(sequence)); f_itr < r_itr; ++f_itr, --r_itr) {
        if (*f_itr != complement(*r_itr)) return false;
    }
    
    return true;
}

template <typename SequenceType>
std::unordered_map<char, std::size_t> count_bases(const SequenceType& sequence)
{
    std::unordered_map<char, std::size_t> result {};
    result.reserve(5); // 4 bases + N
    
    for (char base : sequence) {
        ++result[base];
    }
    
    return result;
}

namespace detail
{
    // Replaces all contiguous sub-sequences of N's with a single N, inplace, and returns a map of
    // each N position in the new sequence, and how many N's have been removed up to the first non-N
    // base past the position
    template <typename SequenceType>
    std::map<std::size_t, std::size_t> collapse_ns(SequenceType& sequence)
    {
        std::map<std::size_t, std::size_t> result {};
        
        auto last = std::end(sequence);
        std::size_t position {}, num_removed {};
        
        for (auto first = std::begin(sequence); first != last;) {
            auto it1 = std::adjacent_find(first, last, [] (char lhs, char rhs) { return lhs == 'N' && lhs == rhs; });
            
            if (it1 == last) break;
            
            auto it2 = std::find_if_not(it1, last, [] (char base) { return base == 'N'; });
            
            position    += std::distance(first, it1);
            num_removed += std::distance(it1, it2) - 1;
            
            result.emplace(position, num_removed);
            
            first = it2;
        }
        
        if (!result.empty()) {
            sequence.erase(std::unique(std::next(std::begin(sequence), std::cbegin(result)->first), last,
                                       [] (char lhs, char rhs) { return lhs == 'N' && lhs == rhs; }), last);
        }
        
        return result;
    }
    
    inline void rebase(std::vector<Tandem::StringRun>& runs, const std::map<std::size_t, std::size_t>& shift_map)
    {
        if (shift_map.empty()) return;
        
        auto shift_map_it = std::cbegin(shift_map);
        for (auto& run : runs) {
            while (std::next(shift_map_it)->first <= run.pos) ++shift_map_it;
            run.pos += static_cast<decltype(run.pos)>(shift_map_it->second);
        }
    }
} // end namespace detail

struct TandemRepeat
{
    using SizeType = GenomicRegion::SizeType;
    TandemRepeat() = delete;
    template <typename T>
    explicit TandemRepeat(T region, GenomicRegion::SizeType period) : region {std::forward<T>(region)}, period {period} {}
    
    GenomicRegion region;
    GenomicRegion::SizeType period;
};

template <typename SequenceType>
std::vector<TandemRepeat> find_exact_tandem_repeats(SequenceType sequence, const GenomicRegion& region,
                                                    GenomicRegion::SizeType min_repeat_size = 2,
                                                    GenomicRegion::SizeType max_repeat_size = 10000)
{
    if (sequence.back() != 'N') {
        sequence.reserve(sequence.size() + 1);
        sequence.push_back('N');
    }
    
    auto n_shift_map = detail::collapse_ns(sequence);
    
    auto maximal_repetitions = Tandem::find_maximal_repetitions(sequence , min_repeat_size, max_repeat_size);
    
    detail::rebase(maximal_repetitions, n_shift_map);
    
    std::vector<TandemRepeat> result {};
    result.reserve(maximal_repetitions.size());
    
    auto offset = region.get_begin();
    
    for (const auto& run : maximal_repetitions) {
        result.emplace_back(GenomicRegion {region.get_contig_name(),
            static_cast<GenomicRegion::SizeType>(run.pos + offset),
            static_cast<GenomicRegion::SizeType>(run.pos + run.length + offset)
        }, run.period);
    }
    
    return result;
}

template <typename SequenceType>
double gc_bias(const SequenceType& sequence)
{
    auto gc_count = std::count_if(std::cbegin(sequence), std::cend(sequence),
                                  [] (char base) { return base == 'G' || base == 'C'; });
    return static_cast<double>(gc_count) / sequence.length();
}

#endif /* defined(__Octopus__sequence_utils__) */