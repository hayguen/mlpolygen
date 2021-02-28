# This file is part of MLPolyGen, a maximal-length polynomial generator
# for linear feedback shift registers.
# 
# Copyright (C) 2012  Gregory E. Allen
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

PAIRTEST_MAX ?= 20
LENTEST_MAX ?= 20
VALTEST_MAX ?= 20

# pair tests look that we get the same answer with/without the -p flag
# len tests verify the the sequence length is correct (vs published info)
# value tests verify that the values in the sequence are correct (vs published info)

all : results.txt
	@cat $<

results.txt : cleanresults pairtests lentests valtests
	@if grep -iq FAILED $@; then \
      echo '###' FAILURE encountered >> $@; \
    else echo "All tests passed" >> $@; fi

# make sure that shell brace expansion works, e.g. {1..20}
SHELL=/bin/bash

#==============================================================================

PAIRTEST_SIZES = $(shell echo {1..$(PAIRTEST_MAX)})
PAIRTEST_FILES = $(patsubst %, files/%.pairtest.txt, $(PAIRTEST_SIZES))
PAIRTEST_SRC1 = $(patsubst %, files/%.mlp.txt, $(PAIRTEST_SIZES))
PAIRTEST_SRC2 = $(patsubst %, files/%.pmlp.txt, $(PAIRTEST_SIZES))
PAIRTEST_ALL = $(PAIRTEST_FILES) $(PAIRTEST_SRC1) $(PAIRTEST_SRC2)
PAIRTEST_ALL += files/pairtests.txt

pairtests : $(PAIRTEST_ALL)
	@cat files/pairtests.txt >> results.txt

clean_pairtests :
	@rm -f $(PAIRTEST_ALL)

MLPOLYGEN=../build/mlpolygen

files/%.mlp.txt :
	$(MLPOLYGEN) $* > $@

files/%.pmlp.txt :
	$(MLPOLYGEN) -p $* > $@

%.pairtest.txt : %.mlp.txt %.pmlp.txt
	sort $*.pmlp.txt | diff $< - > $@ || true

files/pairtests.txt : $(PAIRTEST_FILES)
	@printf '' > $@
	@for sz in $(PAIRTEST_SIZES); do \
      STR="pairs for order $$sz: "; \
      if [ -s files/$$sz.pairtest.txt ]; then \
        echo "$$STR FAILED"; else echo "$$STR passed"; fi \
      >> $@; \
    done

#==============================================================================

LENTEST_SIZES = $(shell echo {1..$(LENTEST_MAX)})
LENTEST_SRC = $(patsubst %, files/%.mlp.txt, $(LENTEST_SIZES))
LENTEST_ALL = $(LENTEST_SRC)
LENTEST_ALL += files/lentests.txt

lentests : $(LENTEST_ALL)
	@cat files/lentests.txt >> results.txt

clean_lentests :
	@rm -f $(LENTEST_ALL) files/seqlen*.txt

# put our seqlen.txt in the same (sensible) format as those published
files/seqlen.txt : $(LENTEST_SRC)
	@printf '' > $@
	@for f in $^; do wc -l $$f >> $@; done
	sed 's/ *\([0-9]*\) files\/\(.*\)\.mlp\.txt/\2 \1/' $@ > $@.tmp
	@mv $@.tmp $@

# get some published answers for the sequence lengths
files/seqlen.ref.txt :
	curl http://oeis.org/A011260/b011260.txt | grep -v '#' > $@

%.sort.txt : %.txt
	sort $^ > $@

files/lentests.txt : files/seqlen.sort.txt files/seqlen.ref.sort.txt
	comm -23 $^ > $@.tmp
	@STR="sequences lengths $(LENTEST_SIZES):"; \
	if [ -s $@.tmp ]; then \
      echo "$$STR FAILED" > $@; cat $@.tmp >> $@; \
    else echo "$$STR passed" >> $@; fi
	@rm -f $@.tmp

#==============================================================================

VALTEST_SIZES = $(shell echo {4..$(VALTEST_MAX)})
VALTEST_FILES = $(patsubst %, files/%.valtest.txt, $(VALTEST_SIZES))
VALTEST_SRC = $(patsubst %, files/%.mlp.txt, $(VALTEST_SIZES))
VALTEST_REF = $(patsubst %, files/%.dat.gz, $(VALTEST_SIZES))
VALTEST_ALL = $(VALTEST_FILES) $(VALTEST_SRC)
VALTEST_ALL += files/valtests.txt

valtests : $(VALTEST_ALL) $(VALTEST_REF)
	@cat files/valtests.txt >> results.txt

clean_valtests :
	@rm -f $(VALTEST_ALL)

realclean_valtests :
	@rm -f $(VALTEST_REF)

# get some published answers for the poly values
files/%.dat.gz :
	curl -o $@ http://www.ece.cmu.edu/~koopman/lfsr/`basename $@`

%.valtest.txt : %.dat.gz %.mlp.txt
	gunzip -c $< | grep -v value | diff -i $*.mlp.txt - > $@ || true

files/valtests.txt : $(VALTEST_FILES)
	@printf '' > $@
	@for sz in $(VALTEST_SIZES); do \
      STR="values for order $$sz: "; \
      if [ -s files/$$sz.valtest.txt ]; then \
        echo "$$STR FAILED"; else echo "$$STR passed"; fi \
      >> $@; \
    done

#==============================================================================

cleanresults :
	@rm -f results.txt
	@mkdir -p files
	rm -f files/pairtests.txt files/lentests.txt files/valtests.txt

clean : cleanresults clean_pairtests clean_lentests clean_valtests

realclean : clean realclean_valtests
