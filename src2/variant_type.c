/*  
    Copyright (C) 2016,2017  BGI Research

    Author: Shi Quan (shiquan@genomics.cn)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE. 
*/

#include "utils.h"
#include <string.h>
#include "htslib/hts.h"
#include "htslib/faidx.h"
#include "variant_type.h"

// Return the location of terminal codon on the sequence.
// Return -1 if no found.
int check_stop_codon(char *seq, char *p_end, int mito)
{
    char *ss = seq;
    char *se = p_end;
    int i, l = 0;
    if ( se == NULL ) {
        for ( se = ss; se != NULL && *se; ++se, ++l );        
    }

    l /= 3;
    
    for ( i = 0; i < l;  ++i) {
        if ( codon2aminoid(ss,mito) == C4_Stop ) {
            return i+1;
        }
        ss += 3;
    }
    return -1;
}

void compl_seq(char *seq, int l)
{
    int i;
    for ( i = 0; i < l/2; i++ ) {
        char c = revseqarr[seq2code4(seq[i])];
        seq[i] = revseqarr[seq2code4(seq[l-i-1])];
        seq[l-i-1] = c;
    }
    if ( l & 1 ) {
        seq[l/2] = revseqarr[seq2code4(seq[l/2])];
    }    
}

int seq2code4(int seq)
{
    static const int seq2num_table[256] = {
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 0, 4, 1, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 0, 4, 1, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    };
    return seq2num_table[seq];
}

int same_DNA_seqs(const char *a, const char *b, int l )
{
    static const int seq2num_table[256] = {
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 0, 4, 1, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 0, 4, 1, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    };

    int i;
    for ( i = 0; i < l; ++i )
        if ( seq2num_table[(int)a[i]] != seq2num_table[(int)b[i]] )
            return 1;

    return 0;
}

// no check the codon length for fast read
int codon2aminoid(char *codon, int mito)
{
    if (mito == 0)
        return codon_matrix[seq2code4(codon[0])][seq2code4(codon[1])][seq2code4(codon[2])];
    else
        return mitomap_codon_matrix[seq2code4(codon[0])][seq2code4(codon[1])][seq2code4(codon[2])];
}

char *rev_seqs(const char *dna_seqs, unsigned long n)
{
    if ( n == 0 )
        return NULL;
    int i;
    char *rev = (char*)calloc(n+1, sizeof(char));
    for ( i = 0; i < n; ++i)
        rev[i] = revseqarr[seq2code4(dna_seqs[n-i-1])];
    rev[n] = '\0';
    return rev;
}

// do NOT check the length of input string for fast access. So it is not memory safe.

/* // define_var_type return the variant type from the transcript block and variants */
/* // only account exon region */
/* // start is 0 based position aligned on the block */
/* // coding_start is 0 based position on block, start coding from this position, sometimes */
/* // block sequence consist of utr and cds sequences */
/* // coding_end is 1 based position on block, end coding till this position */
/* // coding_start should smaller than start because this function only works for cds region */
/* enum var_type check_var_type(char *block, int block_length, int start, char *ref, int ref_length, char *alt, int alt_length ) */
/* { */
/* //    if ( block_length%3 ) */
/*     //      error("transcript block is incomplete. %d.", block_length); */

/*     //int codon_start = start/3; */
/*     //int codon_length = block_length/3; */

/*     //if ( codon_start == 0 || codon_start == codon_length -1 ) */
/*     //return var_is_splice_site; */
    
/*     // deletion */
/*     if ( ref_length > alt_length ) { */
/*         if ( (ref_length - alt_length)%3 ) */
/*             return var_is_frameshift; */
/*         else */
/*             return var_is_inframe_deletion; */
/*     } */
/*     // insertion */
/*     if ( ref_length < alt_length ) { */
/*         if ( (alt_length - ref_length)%3 ) */
/*             return var_is_frameshift; */
/*         else */
/*             return var_is_inframe_insertion; */
/*     } */
    
/*     // snv */
/*     if ( ref_length == 1 ) { */
/*         char codon[4]; */
/*         codon[3] = '\0'; */
/*         memcpy(codon, block + start/3*3, 3); */
/*         if ( codon[start%3] == *alt ) */
/*             return var_is_reference; */
        
/*         int amino_ref, amino_alt; */
/*         amino_ref = codon2aminoid(codon); */
/*         codon[start%3] = *alt; */
/*         amino_alt = codon2aminoid(codon); */
/*         if ( amino_ref == 0 ) { */
/*             if ( amino_alt == 0 ) */
/*                 return var_is_stop_retained; */
/*             else */
/*                 return var_is_stop_lost; */
/*         } */
/*         if ( amino_alt == 0 ) */
/*             return var_is_nonsense; */
        
/*         if ( amino_ref == amino_alt ) */
/*             return var_is_synonymous; */
/*         else */
/*             return var_is_missense; */
        
/*     } else if ( ref_length%3 ) { */
/*         return var_is_frameshift; */
/*     } */
/*     return var_is_complex; */
/* } */

