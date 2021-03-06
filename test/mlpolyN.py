#/usr/bin/env python
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

# This file makes makefiles that can be used to generate a mlpoly
# file of a specified order.
# It splits the output space into as many as 2**16 subspaces
# which can be executed in parallel and then merged together.
# By using sort to perform the merging, we can leverage the
# faster symmetric pairs method.
# This approach provides:
#  - concurrent execution to exploit parallelism, and
#  - simple checkpointing so that the process can be stopped and restarted

import sys

def SegTargetList(polyOrd,totSegOrd,segOrd):
	if segOrd==0: # root name
		return ['mlpoly%d.txt.gz' % polyOrd]
	totNumDigits = (totSegOrd+3)/4
	numXs = (totSegOrd-segOrd)/4
	numDigits = totNumDigits-numXs
	assert (totSegOrd==segOrd) or (totSegOrd-segOrd>=4)
	fmtStr = 'files/mlpoly%d-%%0%dx%s.txt.gz' % (polyOrd, numDigits, 'x'*numXs)
	incr = 2**((totSegOrd-segOrd)%4)
	return [ fmtStr%(x*incr) for x in range(2**segOrd) ]

def PrintSortRules(olist,ilist):
	leno = len(olist)
	leni = len(ilist)
	entriesPerRule = leni/leno
	print '\n# %d sort rule(s) of %d files each' % (leno,entriesPerRule)
	for x in xrange(leno):
		print '%s:' % olist[x],
		subList = ilist[ x*entriesPerRule : x*entriesPerRule+entriesPerRule ]
		print ' '.join(subList)
		print '\tgunzip -c $^ | sort | gzip > $@'

def PrintLeafRules(order, olist):
	numSegments = len(olist)
	print '\n# %d leaf rule(s)' % (numSegments)

	if numSegments==1: # no need for sval, eval, don't want -p
		print '%s:' % olist[0]
		print '\t$(MLPOLYGEN) %d | gzip -c > $@' % order
		return

	totalEndValue = 2**order # actually +1
	totalStartValue = totalEndValue/2
	incr = totalStartValue / numSegments

	for s in xrange(numSegments):
		sval = totalStartValue+s*incr
		eval = sval+incr-1
		print '%s:' % olist[s]
		print '\t$(MLPOLYGEN) -p -s0x%x -e0x%x %d \\' % (sval,eval,order)
		print '    | gzip > $@' 

def main():
	order = int(sys.argv[1])
	segmentsOrder = None
	if len(sys.argv)>=3:
		segmentsOrder = int(sys.argv[2])

	print '### this file generated by: %s' % ' '.join(sys.argv)
	print '#'
	incrPerLevel = 8

	if segmentsOrder is None:
		# make a guess
		if order<=20:
			segmentsOrder = 0
		elif order<=24:
			segmentsOrder = 4
		else:
			segmentsOrder = order-20
			if segmentsOrder>16:
				segmentsOrder = 16
			numLevels = segmentsOrder/9 + 1
			incrPerLevel = (segmentsOrder/numLevels) + 1
			print '# %d levels, %d incrPerLevel\n' % (numLevels,incrPerLevel)
	
	print 'mlpoly%d.txt.gz:' % order
	print '-include common.mk'

	currSegOrder = 0
	currTgtList = SegTargetList(order,segmentsOrder,currSegOrder)
	while currSegOrder<segmentsOrder:
		nextSegOrder = currSegOrder+incrPerLevel
		if (nextSegOrder>segmentsOrder):
			nextSegOrder = segmentsOrder
		nextTgtList = SegTargetList(order,segmentsOrder,nextSegOrder)
		PrintSortRules(currTgtList,nextTgtList)
		currSegOrder = nextSegOrder
		currTgtList = nextTgtList

	PrintLeafRules(order,currTgtList)

	print 'clean-mlpoly%d:' % order
	print '\tfind files -name mlpoly%d-*.txt.gz -print0 | xargs -0 rm' % order
	print '\trm -f mlpoly%d.txt.gz' % order

	print 'clean-files:: clean-mlpoly%d' % order

if __name__ == '__main__':
	main()
