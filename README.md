This is a slightly modified version of https://github.com/lemire/SIMDCompressionAndIntersection 

This includes at least two additions/changes:

1) Additional codec "ironpeter" Originally developed by Peter Popov: https://github.com/IronPeter/groupvarint/blob/master/compressor.cpp

2) A utility testcodecs carries tests in both cache-to-cache and mem-to-cache mode. So, one could clearly see that reading compressed integers can be 1.5-2 times as fast compared to reading uncompressed integers from memory.


Try, e.g.:

make  
./testcodecs copy s4-bp128-d4 s4-fastpfor-d4 
