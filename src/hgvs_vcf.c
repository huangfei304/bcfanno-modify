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
#include "hgvs.h"
#include "hgvs_vcf.h"
#include "genepred.h"
#include "htslib/hts.h"
#include "htslib/vcf.h"
#include "htslib/kseq.h"
#include "htslib/kstring.h"
#include <string.h>


static int init_flag = 0;

int hgvs_update_vcf_header(bcf_hdr_t *hdr)
{
    int id;

    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "Gene");
    if (id == -1) {
	bcf_hdr_append(hdr, "##INFO=<ID=Gene,Number=A,Type=String,Description=\"Gene names\">");
	bcf_hdr_sync(hdr);
	id = bcf_hdr_id2int(hdr, BCF_DT_ID, "Gene");
	assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }    

    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "Transcript");
    if (id == -1) {
	bcf_hdr_append(hdr, "##INFO=<ID=Transcript,Number=A,Type=String,Description=\"Transcript names\">");
	bcf_hdr_sync(hdr);
	id = bcf_hdr_id2int(hdr, BCF_DT_ID, "Transcript");
	assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }

    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "HGVSnom");
    if (id == -1) {
	bcf_hdr_append(hdr, "##INFO=<ID=HGVSnom,Number=A,Type=String,Description=\"HGVS nomenclature for the description of DNA sequence variants\">");
	bcf_hdr_sync(hdr);
	id = bcf_hdr_id2int(hdr, BCF_DT_ID, "HGVSnom");
	assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }

    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "IVSnom");
    if (id == -1) {
        bcf_hdr_append(hdr, "##INFO=<ID=IVSnom,Number=A,Type=String,Description=\"Old style nomenclature for the description of intron variants. Not recommand to use it in normal practice.\">");
        bcf_hdr_sync(hdr);
        id = bcf_hdr_id2int(hdr, BCF_DT_ID, "IVSnom");
        assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }

    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "Oldnom");
    if (id == -1) {
	bcf_hdr_append(hdr, "##INFO=<ID=Oldnom,Number=A,Type=String,Description=\"Old style nomenclature for the description of variants. Based on the gene position. Compared with HGVS nomenclature, Oldnom also count UTR5 length for coding transcript. For noncoding transcript, HGVSnom is same with Oldnom. For intron, c.IVS notation will be annotated.\">");
bcf_hdr_sync(hdr);
	id = bcf_hdr_id2int(hdr, BCF_DT_ID, "Oldnom");
	assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }

    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "AAlength");
    if (id == -1) {
	bcf_hdr_append(hdr, "##INFO=<ID=AAlength,Number=A,Type=String,Description=\"Amino acid length for each transcript. For noncoding transcript AAlength should always be 0.\">");
	bcf_hdr_sync(hdr);
	id = bcf_hdr_id2int(hdr, BCF_DT_ID, "AAlength");
	assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }    
    
    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "ExonIntron");
    if (id == -1) {
        bcf_hdr_append(hdr, "##INFO=<ID=ExonIntron,Number=A,Type=String,Description=\"Exon/CDS or intron id on transcripts.\">");
        bcf_hdr_sync(hdr);
        id = bcf_hdr_id2int(hdr, BCF_DT_ID, "ExonIntron");
        assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }

    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "FlankSeq");
    if (id == -1) {
        bcf_hdr_append(hdr, "##INFO=<ID=FlankSeq,Number=1,Type=String,Description=\"Three nearby bases of current position.\">");
        bcf_hdr_sync(hdr);
        id = bcf_hdr_id2int(hdr, BCF_DT_ID, "FlankSeq");
        assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }
    
    id = bcf_hdr_id2int(hdr, BCF_DT_ID, "VarType");
    if (id == -1) {
	bcf_hdr_append(hdr, "##INFO=<ID=VarType,Number=A,Type=String,Description=\"Variant type.\">");
	bcf_hdr_sync(hdr);
	id = bcf_hdr_id2int(hdr, BCF_DT_ID, "VarType");
	assert(bcf_hdr_idinfo_exists(hdr, BCF_HL_INFO, id));
    }
    
    return 0;
}

int init_hgvs_anno(const char *data, const char *fasta, bcf_hdr_t *hdr)
{
    if ( init_hgvs_spec(data, fasta) )
        return 1;

    hgvs_update_vcf_header(hdr);


    // turn init flag on
    init_flag = 1;
    
    return 0;
}

int close_hgvs_anno()
{
    if ( init_flag == 1 )
        hgvs_spec_destroy();
    
    return 0;
}

static char *retrieve_gene_des(struct hgvs_des *des)
{
    kstring_t string = { 0, 0, 0};

    int i;
    for ( i = 0; i < des->l; ++i ) {
        struct hgvs_name *name = &des->a[i].name;
        if ( i ) kputc('|', &string);
        kputs(name->name2, &string);
    }

    return string.s;
}

static char *retrieve_trans_des(struct hgvs_des *des)
{
    kstring_t string = { 0, 0, 0};

    int i;
    for ( i = 0; i < des->l; ++i ) {
        struct hgvs_name *name = &des->a[i].name;
        if ( i ) kputc('|', &string);
        kputs(name->name1, &string);
            {
                int l = strlen(name->name1);
                int k;
                for ( k = 0; k < l; ++k )                    
                    if ( name->name1[k] == '.')
                        break;
                if ( k == l && name->name_version > 0 ) {
                    ksprintf(&string, ".%d",name->name_version);
                }                
            }
    }
    return string.s;
}

static char *retrieve_hgvs_des(struct hgvs_des *des)
{
    kstring_t string = { 0, 0, 0};
    int i;
    for ( i = 0; i < des->l; ++i ) {
        struct hgvs_name *name = &des->a[i].name;
        struct var_func_type *type = &des->a[i].type;
        if ( i ) kputc('|', &string);
        ksprintf(&string, "%s",name->name1);
        //char *ss = name->name1;
            {
                int l = strlen(name->name1);
                int k;
                for ( k = 0; k < l; ++k )                    
                    if ( name->name1[k] == '.')
                        break;
                if ( k == l && name->name_version > 0 ) {
                    ksprintf(&string, ".%d",name->name_version);
                }
            }
        kputc(':', &string);
        if ( type->func == func_region_noncoding ) {
            kputs("n.", &string);
        } else if ( type->func == func_region_cds ) {
            kputs("c.", &string);
        } else if ( type->func == func_region_utr5 ) {
            kputs("c.-", &string);
        } else if ( type->func == func_region_utr3 ) {
            kputs("c.*", &string);
        }
        ksprintf(&string, "%d", name->loc);
        if ( name->offset > 0 ) {
            ksprintf(&string, "+%d", name->offset);
        } else if (name->offset < 0) {
            ksprintf(&string, "%d", name->offset);
        }
        if ( des->start != des->end ) {
            kputc('_', &string);
            if ( type->func == func_region_utr5 ) {
                kputc('-', &string);
            } else if ( type->func == func_region_utr3 ) {
                kputc('*', &string);
            }
            ksprintf(&string, "%d", name->end_loc);
            if ( name->end_offset > 0 ) {
                ksprintf(&string, "+%d", name->end_offset);
            } else if ( name->end_offset < 0) {
                ksprintf(&string, "%d", name->end_offset);
            }            
        }
        
        if ( des->ref_length == 0 ) {
            if ( name->strand == '+' ) {
                ksprintf(&string, "ins%s", des->alt);
            } else {
                char *rev = rev_seqs(des->alt, des->alt_length);
                ksprintf(&string, "ins%s", rev);
                free(rev);
            }
        } else if ( des->alt_length == 0 ) {
            if ( name->strand == '+' ) {
                ksprintf(&string, "del%s", des->ref);
            } else {
                char *rev = rev_seqs(des->ref, des->ref_length);
                ksprintf(&string, "del%s", rev);
                free(rev);                
            }
        } else {
            if ( name->strand == '+' ) {
                ksprintf(&string, "%s>%s", des->ref, des->alt);
            } else {
                char *ref = rev_seqs(des->ref, des->ref_length);
                char *alt = rev_seqs(des->alt, des->alt_length);
                ksprintf(&string, "%s>%s", ref, alt);
                free(ref);
                free(alt);
            }
        }
        // protein code for snp
        if ( type->loc_amino > 0 && des->type == var_type_snp ) {
            ksprintf(&string, "(p.%s%d%s/p.%s%d%s)", codon_names[type->ori_amino], type->loc_amino, codon_names[type->mut_amino],
                     codon_short_names[type->ori_amino], type->loc_amino, codon_short_names[type->mut_amino]);
        }
        // indels
        else {
            if ( type->vartype == var_is_inframe_insertion ) {
                ksprintf(&string, "(p.%s%d_%s%dins",codon_names[type->ori_amino], type->loc_amino, codon_names[type->ori_end_amino], type->loc_end_amino);
                int i;
                for (i = 0; i < type->n; ++i)
                    kputs(codon_names[type->aminos[i]], &string);
                kputc(')', &string);                         
            }
            else if ( type->vartype == var_is_inframe_deletion ) {
                if ( type->loc_end_amino == 0 ) {
                    ksprintf(&string, "(p.%s%ddel",codon_names[type->ori_amino], type->loc_amino);
                }
                else {
                    ksprintf(&string, "(p.%s%d_%s%ddel",codon_names[type->ori_amino], type->loc_amino, codon_names[type->ori_end_amino], type->loc_end_amino);
                }
                int i;
                for (i = 0; i < type->n; ++i)
                    kputs(codon_names[type->aminos[i]], &string);
                kputc(')', &string);                                         
            }
            else if ( type->vartype == var_is_inframe_delins ) {
                if ( type->loc_end_amino == 0 ) {
                    ksprintf(&string, "(p.%s%ddelins",codon_names[type->ori_amino], type->loc_amino);
                }
                else {
                    ksprintf(&string, "(p.%s%d_%s%ddelins",codon_names[type->ori_amino], type->loc_amino, codon_names[type->ori_end_amino], type->loc_end_amino);
                }
                int i;
                for (i = 0; i < type->n; ++i)
                    kputs(codon_names[type->aminos[i]], &string);
                kputc(')', &string);                                         
            }
            else if ( type->vartype == var_is_frameshift ) {
                ksprintf(&string, "(p.%s%d%sfs",codon_names[type->ori_amino], type->loc_amino, codon_names[type->mut_amino]);
                if ( type->fs == -1 )
                    kputc('*', &string);
                else
                    kputw(type->fs, &string);
                kputc(')', &string);                                         
            }
        }
    }
    return string.s;
}

static char *retrieve_oldnom_des(struct hgvs_des *des)
{
    kstring_t string = { 0, 0, 0};
    int i, j;
    for ( i = 0, j = 0; i < des->l; ++i ) {
        struct hgvs_name *name = &des->a[i].name;
        struct var_func_type *type = &des->a[i].type;
        if ( type->func == func_region_noncoding )
            continue;
        
        if ( j ) kputc('|', &string);
        j++;
        
        ksprintf(&string, "%s",name->name1);
        if ( name->name_version > 0 ) {
            ksprintf(&string, ".%d",name->name_version);
        }
        kputc(':', &string);
        //if ( name->offset == 0 ) {
        ksprintf(&string, "n.%d", name->pos);
        if ( name->offset > 0 ) 
            ksprintf(&string, "+%d", name->offset);
        else if (name->offset < 0 )
            kputw(name->offset, &string);
        
        if ( des->start != des->end ) {
                kputc('_', &string);
                ksprintf(&string, "%d", name->end_pos);
                if ( name->end_offset > 0 ) 
                    ksprintf(&string, "+%d", name->end_offset);
                else if (name->end_offset < 0 )
                    kputw(name->end_offset, &string);
        }
        /* // c.IVS nomen */
        /* else { */
        /*     ksprintf(&string, "c.IVS%d", type->count); */
        /*     if ( name->offset > 0 ) { */
        /*         ksprintf(&string, "+%d", name->offset); */
        /*     } else if (name->offset < 0) { */
        /*         ksprintf(&string, "%d", name->offset); */
        /*     } */
        /*     if ( des->start != des->end ) { */
        /*         kputc('_', &string); */
        /*         if ( name->end_offset == 0 ) { */
        /*             kputc('?', &string); */
        /*         } else if ( name->end_offset > 0 ) { */
        /*             ksprintf(&string, "+%d", name->end_offset); */
        /*         } else if ( name->end_offset < 0) { */
        /*             ksprintf(&string, "%d", name->end_offset); */
        /*         }             */
        /*     }             */
        /* } */
            
        if ( des->ref_length == 0 ) {
            if ( name->strand == '+') {
                ksprintf(&string, "ins%s", des->alt);
            } else {
                char *rev = rev_seqs(des->alt, des->alt_length);
                ksprintf(&string, "ins%s", rev);
                free(rev);
            }
        } else if ( des->alt_length == 0 ) {
            if ( name->strand == '+' ) {
                ksprintf(&string, "del%s", des->ref);
            } else {
                char *rev = rev_seqs(des->ref, des->ref_length);
                ksprintf(&string, "del%s", rev);
                free(rev);                
            }
        } else {
            if ( name->strand == '+') {
                ksprintf(&string, "%s>%s", des->ref, des->alt);
            } else {
                char *ref = rev_seqs(des->ref, des->ref_length);
                char *alt = rev_seqs(des->alt, des->alt_length);
                ksprintf(&string, "%s>%s", ref, alt);
                free(ref);
                free(alt);
            }
        }
    }
    return string.s;
}

static char *retrieve_exonintron_des(struct hgvs_des *des)
{
    kstring_t str = { 0, 0, 0};
    int i;
    
    for ( i = 0; i < des->l; ++i ) {
        struct hgvs_name *name = &des->a[i].name;
        struct var_func_type *type = &des->a[i].type;

        if ( i ) kputc('|', &str);
        if ( name->offset != 0 ) {
            ksprintf(&str,"I%d",type->count);
        } else {
            ksprintf(&str, "E%d",type->count);
            if ( type->count2 != 0 )                 
                ksprintf(&str, "/C%d", type->count2);
        }
        
    }
    return str.s;
}

static char *retrieve_ivsnom_des(struct hgvs_des *des)
{
    kstring_t string = { 0, 0, 0};
    int i, j;
    for ( i = 0, j = 0; i < des->l; ++i ) {
        struct hgvs_name *name = &des->a[i].name;
        struct var_func_type *type = &des->a[i].type;
        if ( name->offset == 0 )
            continue;
        if ( j ) kputc('|', &string);
        ++j;
        
        ksprintf(&string, "%s",name->name1);
        if ( name->name_version > 0 ) {
            ksprintf(&string, ".%d",name->name_version);
        }
        kputc(':', &string);
        ksprintf(&string, "c.IVS%d", type->count);
        if ( name->offset > 0 ) {
            ksprintf(&string, "+%d", name->offset);
        } else if (name->offset < 0) {
            ksprintf(&string, "%d", name->offset);
        }
        if ( des->start != des->end ) {
            kputc('_', &string);
            if ( name->end_offset == 0 ) {
                kputc('?', &string);
            } else if ( name->end_offset > 0 ) {
                ksprintf(&string, "+%d", name->end_offset);
            } else if ( name->end_offset < 0) {
                ksprintf(&string, "%d", name->end_offset);
            }            
        }
        
        if ( des->ref_length == 0 ) {
            if ( name->strand == '+' || name->offset != 0) {
                ksprintf(&string, "ins%s", des->alt);
            } else {
                char *rev = rev_seqs(des->alt, des->alt_length);
                ksprintf(&string, "ins%s", rev);
                free(rev);
            }
        } else if ( des->alt_length == 0 ) {
            if ( name->strand == '+' || name->offset != 0 ) {
                ksprintf(&string, "del%s", des->ref);
            } else {
                char *rev = rev_seqs(des->ref, des->ref_length);
                ksprintf(&string, "del%s", rev);
                free(rev);                
            }
        } else {
            if ( name->strand == '+' || name->offset != 0) {
                ksprintf(&string, "%s>%s", des->ref, des->alt);
            } else {
                char *ref = rev_seqs(des->ref, des->ref_length);
                char *alt = rev_seqs(des->alt, des->alt_length);
                ksprintf(&string, "%s>%s", ref, alt);
                free(ref);
                free(alt);
            }
        }
    }
    return string.s;
}
static char *retrieve_aalength_des(struct hgvs_des *des)
{
    kstring_t str = { 0, 0, 0};
    int i;
    for ( i = 0; i < des->l; ++i ) {
        struct hgvs_name *name = &des->a[i].name;
        if ( i )
            kputc('|', &str);
        kputw(name->aa_length, &str);
    }
    return str.s;
}
static char *retrieve_vartype_des(struct hgvs_des *des)
{
    kstring_t string = { 0, 0, 0};

    int i;
    for ( i = 0; i < des->l; ++i ) {
        struct var_func_type *type = &des->a[i].type;
        if ( i ) kputc('|', &string);
        // update 2017/12/01: splice site nomencluture
        if ( type->vartype2 != var_is_not_splice )  {
            kputs(var_type_splice_string(type->vartype2), &string);
        
            if ( type->vartype != var_is_intron && type->vartype != var_is_reference)
                ksprintf(&string, "(%s)", var_type_string(type->vartype));
        }
        else {
            kputs(var_type_string(type->vartype), &string);
        }
            
    }
    return string.s;
}
// 0 on success, 1 on failed, 2 on NOT inited.
int setter_hgvs_vcf(bcf_hdr_t *hdr, bcf1_t *line)
{
    // If not inited, skip.
    if ( init_flag == 0 )
        return 2;
    
    int i;
    kstring_t gene = { 0, 0, 0};
    kstring_t transcript = { 0, 0, 0};
    kstring_t hgvs_nom = { 0, 0, 0};
    kstring_t vartype = { 0, 0, 0};
    kstring_t exon_id = {0, 0, 0};
    kstring_t ivs_nom = { 0, 0, 0};
    kstring_t old_nom = { 0, 0, 0};
    kstring_t aa_length = { 0, 0, 0};
    
    int is_empty = 1;
    for ( i = 1; i < line->n_allele; ++i ) {
        // if more than one allele, seperated with ','
        if ( i > 1 ) {
            kputc(',', &gene);
            kputc(',', &transcript);
            kputc(',', &hgvs_nom);
            kputc(',', &exon_id);
            kputc(',', &ivs_nom);
            kputc(',', &old_nom);
            kputc(',', &aa_length);
            kputc(',', &vartype);
        }
        // deletion skips
        if ( line->d.allele[i] == NULL || *line->d.allele[i] == '*' || seq2code4(*line->d.allele[i]) == 4) 
	    goto empty_alleles;
        
        const char *name = bcf_hdr_id2name(hdr, line->rid);        
        setter_description(name, line->pos+1, line->d.allele[0], line->d.allele[i]);
        struct hgvs_des *des = fill_hgvs_name();            
        char *gene_string = retrieve_gene_des(des);
        char *trans_string = retrieve_trans_des(des);
        char *hgvs_string = retrieve_hgvs_des(des);
        char *vartype_string = retrieve_vartype_des(des);
        char *oldnom_string = retrieve_oldnom_des(des);
        char *exonintron_string = retrieve_exonintron_des(des);
        char *ivsnom_string = retrieve_ivsnom_des(des);
        char *aalength_string = retrieve_aalength_des(des);
        
        hgvs_des_clear(des);
        if ( gene_string == NULL ) {
          empty_alleles:
            kputs(".", &gene);
            kputs(".", &transcript);
            kputs(".", &hgvs_nom);
            kputs(".", &vartype);
            kputs(".", &exon_id);
            kputs(".", &ivs_nom);
            kputs(".", &old_nom);
            kputs(".", &aa_length);
        }
        else {
            is_empty = 0;
            kputs(gene_string, &gene);
            kputs(trans_string, &transcript);
            kputs(hgvs_string, &hgvs_nom);
            kputs(vartype_string, &vartype);

            if ( oldnom_string != NULL)
                kputs(oldnom_string, &old_nom);
            else
                kputc('.', &old_nom);
            
            kputs(exonintron_string, &exon_id);

            if ( ivsnom_string != NULL ) 
                kputs(ivsnom_string, &ivs_nom);
            else
                kputc('.', &ivs_nom);
            
            kputs(aalength_string, &aa_length);
            
            free(gene_string);
            free(trans_string);
            free(hgvs_string);
            free(vartype_string);

            if ( oldnom_string != NULL)
                free(oldnom_string);

            free(exonintron_string);

            if ( ivsnom_string != NULL)
                free(ivsnom_string);

            free(aalength_string);
        }        
    }
    if ( i > 1 && is_empty == 0) {
        bcf_update_info_string(hdr, line, "Gene", gene.s);
        bcf_update_info_string(hdr, line, "Transcript", transcript.s);
        bcf_update_info_string(hdr, line, "HGVSnom", hgvs_nom.s);
        bcf_update_info_string(hdr, line, "VarType", vartype.s);
        if ( ivs_nom.l > 1 ) 
            bcf_update_info_string(hdr, line, "IVSnom", ivs_nom.s); 

        if ( old_nom.l > 1) 
            bcf_update_info_string(hdr, line, "Oldnom", old_nom.s);
        
        bcf_update_info_string(hdr, line, "AAlength", aa_length.s);
        bcf_update_info_string(hdr, line, "ExonIntron", exon_id.s);
    }

    free(gene.s);
    free(transcript.s);
    free(hgvs_nom.s);
    free(vartype.s);
    free(ivs_nom.s);
    free(old_nom.s);
    free(aa_length.s);
    free(exon_id.s);
    return 0;                  
}


#ifdef HGVS_VCF_MAIN
int usage()
{
    fprintf(stderr, "hgvs_vcf_anno -data genepred.tsv.gz -rna rna.fa.gz in.vcf\n");
    return 1;
}

struct args {
    const char *input_fname;
    const char *data_fname;
    const char *rna_fname;
} args = {
    .input_fname = NULL,
    .data_fname = NULL,
    .rna_fname = NULL,
};

int parse_args(int argc, char **argv)
{
    int i;
    if (argc == 1)
        return usage();
    
    for ( i = 1; i < argc; ) {
        const char *a = argv[i++];
        const char **var = NULL;
        if ( strcmp(a, "-h") == 0 )
            return usage();

        if ( strcmp(a, "-data") == 0 && args.data_fname == NULL )
            var = &args.data_fname;
        else if ( strcmp(a, "-rna") == 0 && args.rna_fname == NULL )
            var = &args.rna_fname;

        if ( var != 0 ) {
            if ( i == argc ) {
                error_print("Missing an argument after %s.", a);
                return 1;
            }
            *var = argv[i++];
            continue;
        }

        if ( args.input_fname == NULL ) {
            args.input_fname = a;
            continue;
        }
        error_print("Unknown argument %s.", a);
        return 1;        
    }

    if ( args.input_fname == NULL && (!isatty(fileno(stdin))) )
        args.input_fname = "-";

    if ( args.input_fname == NULL )
        return usage();

    if ( args.data_fname == NULL ) {
        error_print("-data parameter is mandontory.");
        return 1;
    }

    if ( args.rna_fname == NULL ) {
        error_print("-rna parameter is mandontory.");
        return 1;
    }
    
    return 0;
}

int hgvs_vcf_anno()
{
    htsFile *fp = NULL;
    fp = hts_open(args.input_fname, "r");
    if ( fp == NULL ) {
        error_print("%s : %s.", args.input_fname, strerror(errno));
        return 1;
    }
    htsFormat type = *hts_get_format(fp);
    if ( type.format != vcf && type.format != bcf ) {
        error_print("Unsupported input format. Accept BCF/VCF format only. %s.", args.input_fname);
        return 1;
    }

    bcf_hdr_t *hdr = bcf_hdr_read(fp);
    if ( hdr == NULL ) {
        error_return("Failed to parse header of input.");
        return 1;
    }
    bcf_hdr_t *hdr_out = bcf_hdr_dup(hdr);

    if ( init_hgvs_anno(args.data_fname, args.rna_fname, hdr_out) )
        goto failed_init;
    
    htsFile *fp_out = hts_open("-", "w");
    bcf_hdr_write(fp_out, hdr_out);
    
    bcf1_t *line = bcf_init();

    for ( ;; ) {
        if ( bcf_read(fp, hdr, line) )
            break;
        if ( line->rid == -1 )
            continue;
        // do nothing for reference positions
        if ( bcf_get_variant_types(line) != VCF_REF )
            setter_hgvs_vcf(hdr_out, line);
        bcf_write(fp_out, hdr_out, line);
    }

    close_hgvs_anno();
    bcf_destroy(line);
    bcf_hdr_destroy(hdr);
    bcf_hdr_destroy(hdr_out);
    hts_close(fp);
    hts_close(fp_out);

    return 0;

  failed_init:
    bcf_hdr_destroy(hdr);
    bcf_hdr_destroy(hdr_out);
    return 1;
}
int main(int argc, char **argv)
{
    if ( parse_args(argc, argv) )
        return 1;
    if ( hgvs_vcf_anno() )
        return 1;

    return 0;
}


#endif
