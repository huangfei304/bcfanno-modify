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

// init two columns table file, and convert first column name to second column
// file format:  name_old\tname_new
#include "utils.h"
#include "table2hash.h"
#include "htslib/khash.h"
#include "htslib/kseq.h"
#include "htslib/kstring.h"
#include "htslib/bgzf.h"

#define KSTRING_INIT { 0, 0, 0 }
                     
typedef char* string;

KHASH_MAP_INIT_STR(name, string)
KSTREAM_INIT(BGZF*, bgzf_read, 8192)

typedef kh_name_t nameHash;

static nameHash *hash = NULL;

static int table_init_flag = 0;

void table_set_flag()
{
    table_init_flag = 1;
}
void table_clear_flag()
{
    table_init_flag = 0;
}

int table_check_flag()
{
    return table_init_flag == 1;
}

int table_read(const char *fname)
{
    BGZF *fp = bgzf_open(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s : %s.\n", fname, strerror(errno));
        return 1;
    }
    hash = kh_init(name);

    kstring_t string = KSTRING_INIT;
    int dret; 
    kstream_t *ks;
    ks = ks_init(fp);
    while (ks_getuntil(ks, KS_SEP_LINE, &string, &dret) >= 0) {
        if ( string.l == 0 )
            continue;
        if ( string.s[0] == '#')
            continue;
        int nfields = 0;
        int *splits = ksplit(&string, 0, &nfields);
        if ( splits == NULL )
            continue;
        // skip if smaller than two columns
        if ( nfields < 2 )
            continue;
        khint_t k;
        char *name1 = strdup(string.s + splits[0]);
        char *name2 = strdup(string.s + splits[1]);
        k = kh_get(name, hash, name1);
        if ( k == kh_end(hash) ) {
            int ret;
            k = kh_put(name, hash, name1, &ret);
            kh_val(hash, k) = name2;
        } else {
            fprintf(stderr,"Duplicate name, %s.", name1);
            free(name1);
            free(name2);            
        }        
    }
    ks_destroy(ks);
    bgzf_close(fp);
    if ( string.m )
        free(string.s);
    return 0;
}
// return hash value of name, or just return name if name is not a key
char *table_convert_name(char *name)
{
    if ( !table_check_flag() )
        return name;
    
    khint_t k;
    k = kh_get(name, hash, name);
    if (k == kh_end(hash) )
        return name;
    return kh_val(hash, k);
}

int table_release()
{
    if ( !table_check_flag())
        return 0;
    khint_t k;
    for (k = kh_begin(hash); k != kh_end(hash); k++) {
        if (!kh_exist(hash, k))
            continue;
        free(kh_val(hash, k));
        kh_del(name, hash, k);
    }
    kh_destroy(name, hash);
    table_clear_flag();
    return 0;
}

#ifdef _TABLE_HASH_MAIN
int main(int argc, char **argv)
{
}
#endif
