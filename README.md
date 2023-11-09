bcfanno-modify
================

bcfanno-modify is based on [bcfanno](https://github.com/shiquan/bcfanno),  and make some patches. as follows:

```
1. annotation type change. e.g.:
	raw：StartLoss		now: StartLost
	raw: SpliceSites		now: SpliceSite
2. Amino acid annotation. e.g.:
	raw: p.104_105delinsMet		now: p.Thr104_Leu105delinsMet
3. frame-shift in amino acid. e.g.:
	raw: p.Tyr137*fs1		now: p.Tyr137Ter
4. indel annotation.
	e.g.: chr7:103989356:T>TGCCGCCGCC
	raw: RELN c.-1_1del		now: RELN c.-9_-1insGCCGCCGCC
5. ? in bcfanno for frameshift mutation.
6. Delins
	raw: c.41delCinsTT		now: c.41delinsTT
	raw: p*135del			now: p.Arg135Leu
......
```

## Installing bcfanno or bcfanno-modify

Download the source code or binary distribution from https://github.com/shiquan/bcfanno/releases

Or cloning from github, get the most updated version :
```
1. bcfanno
git clone https://github.com/shiquan/bcfanno
cd bcfanno && make
2. bcfanno-modify
git clone git@github.com:huangfei304/bcfanno-modify.git
cd bcfanno-modify && make
## will get bcfanno #####
```

## Test toy data

Run bcfanno with toy data :

```
# stdout output
./bcfanno -c toy.json example/toy.vcf.gz

# Or save results to a compressed VCF :
./bcfanno -c toy.json example/toy.vcf.gz -O z -o results.vcf.gz

# Or save as bcf :
./bcfanno -c toy.json example/toy.vcf.gz -O b -o results.bcf
```


## LICENSE
The full package of bcfanno is distributed by MIT License, copyright 2016-2018 BGI Research.

Belowed package or source code used in bcfanno copyrighted by other institution.
- [htslib1.6](www.htslib.org)  The MIT/Expat License, Copyright (C) 2012-2014 Genome Research Ltd.
- thread_pool.[ch] The MIT/Expat License, Copyright (c) 2013-2017 Genome Research Ltd.


## Reference
1. [HGVS nomenclature](http://varnomen.hgvs.org/)
2. [VCF format](http://www.internationalgenome.org/wiki/Analysis/Variant%20Call%20Format/vcf-variant-call-format-version-41/)
