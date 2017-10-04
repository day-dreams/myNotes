sloppy counter和traditonal counter的比较.

可以看出,sloppycounter的性能明显好于traditionalcounter.

|线程数量|add次数|sloppy counter用时|traditional counter用时|
|:-:|:-:|:-:|:-:|
|1|10000000|0.130323|0.327724|
|2|10000000|0.304245|2.70485 |
|3|10000000|0.387101|3.58324 |
|4|10000000|0.730623|6.92834 |
|5|10000000|0.876632|8.56112|
