# Experiments with Radix Sorting Strings in C and C++

Various MSD radix sorting implementations, for fun and evaluation.

The plan is for some to eventually join my [Notes on Radix Sorting and Implementation](https://github.com/eloj/radix-sorting) repository.

All code is provided under the [MIT License](LICENSE).

## Variant synopsis

* `CE` is for Count, External memory.
* `CI` is for Count, In-place.
* `CB` have merged C (count) and B (bucket) arrays.
* `BM` Use bitmaps for loop iteration.

## Notes

None of the current implementations are really competitive, especially considering the extra space for the
non-inplace variants.
For sure, the memory access pattern is suboptimal for modern architectures, especially at relatively low
radixes (i.e 8 bits).

## <a name="resources">Sources</a>

* \[TM18\] Timo Bingmann, "[Scalable String and Suffix Sorting](https://arxiv.org/abs/1808.00963)", 2018.
