# Experiments with Radix Sorting Strings in C and C++

Various MSD radix sorting implementations, for fun and evaluation.

The plan is for some to eventually join my [Notes on Radix Sorting and Implementation](https://github.com/eloj/radix-sorting) repository.

All code is provided under the [MIT License](LICENSE).

## Variant synopsis

* `CE` is for Count, External memory.
* `CI` is for Count, In-place.
* `CB` have merged C (count) and B (bucket) arrays.
* `BM` Use bitmaps for loop iteration.

## <a name="resources">Sources</a>

* \[TM18\] Timo Bingmann, "[Scalable String and Suffix Sorting](https://arxiv.org/abs/1808.00963)", 2018.
