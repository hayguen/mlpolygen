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

# This makfile (along with helper python script mlpolyN.py) can make
# an output file of a given order (e.g. 32) as follows:
## cd mlpolygen/test
## make -f mlpoly32.txt
# The file mlpoly32.mk will be generated (by the python script),
# containing rules to generate mlpoly32.txt.

MLPOLYGEN=../build/mlpolygen

mlpoly%.mk : mlpolyN.py common.mk
	python $< $* > $@

mlpoly%.txt.gz : mlpoly%.mk
	mkdir -p files
	$(MAKE) -f $^

# make sure that shell brace expansion works, e.g. {1..20}
SHELL=/bin/bash

TEST_MAX ?= 24

TEST_SIZES = $(shell echo {1..$(TEST_MAX)})
TEST_FILES = $(patsubst %, mlpoly%.txt.gz, $(TEST_SIZES))
TEST_MKFS = $(patsubst %, mlpoly%.mk, $(TEST_SIZES))

test: results.txt $(TEST_FILES)
	@cat $<

clean: $(TEST_MKFS)
	for file in $^; do $(MAKE) -f $$file clean-files; done
	rm -rf files lengths*.txt

gzcheck: # check that all .gz files are valid
	find files -name mlpoly\*.txt.gz -print0 | xargs -0 gunzip -t

# everything below here is to verify the sequence lengths and generate results.txt
files/lengths.raw.txt : $(TEST_FILES)
	@echo making $@
	@printf '' > $@
	@for f in $^; do gunzip -c $$f | wc -l >> $@; done

files/lengths.txt : files/lengths.raw.txt
	@echo making $@
	@order=1; while read line; do echo $$order $$line; order=$$((order+1)); done < $< > $@

# get some published answers for the sequence lengths
b011260.txt : 
	curl http://oeis.org/A011260/b011260.txt -o $@
files/lengths.ref.txt : b011260.txt
	mkdir -p files
	cp $< $@

comm23_unsorted = comm -23 $(foreach f,$(1),<(sort $(f)))

files/lengths.err.txt : files/lengths.txt files/lengths.ref.txt
	@echo making $@
	@$(call comm23_unsorted,$^) > $@

results.txt : files/lengths.err.txt cleanresults
	@STR="sequence lengths 1 to $(TEST_MAX):"; \
	if [ -s $< ]; then \
      echo "$$STR FAILED" > $@; cat $< >> $@; \
    else echo "$$STR passed" >> $@; fi

cleanresults :
	@rm -f results.txt
