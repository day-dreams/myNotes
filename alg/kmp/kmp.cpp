class Solution
{
  public:
    bool same_substr(const char *target, int a_begin, int a_end, int b_begin, int b_end)
    {
        if (a_end - a_begin != b_end - b_begin)
        {
            return false;
        }

        while (a_begin < a_end)
        {
            if (target[a_begin] == target[b_begin])
            {
                ++a_begin;
                ++b_begin;
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    /*
     * @param source: source string to be scanned.
     * @param target: target string containing the sequence of characters to
     * match
     * @return: a index to the first occurrence of target in source, or -1  if
     * target is not part of source.
     */
    int strStr(const char *source, const char *target)
    {
        // write your code here

        if (source == nullptr || target == nullptr)
            return -1;

        int  pos         = 0;
        int  target_size = 0, source_size = 0;
        int *k;

        // init target size
        for (auto begin = target; *begin != '\0'; ++begin)
            ++target_size;

        // init source size
        for (auto begin = source; *begin != '\0'; ++begin)
            ++source_size;

        // update k[]
        k = new int[target_size]();
        for (int i = 0; i != target_size; ++i)
        {
            // 计算:在target[i]出发生不匹配时,下次比对的开始位置
            if (i == 0 || i == 1)
                k[i] = 0;
            else
            {
                int refind = 0;

                for (int j = 0; j < i - 1; ++j)
                {
                    bool flag;  // when j!=i-1 ,if [0,j]==[i-j-1,i-1]

                    if (same_substr(target, 0, j + 1, i - j - 1, i))
                    {
                        refind = j + 1;
                    }
                }

                k[i] = refind;
            }
        }

        // for (int i = 0; i != target_size; ++i)
        // 	std::cout << k[i] << ' ';
        // std::cout << std::endl;
        // return 0;

        int s_ite = 0;
        int t_ite = 0;
        while (s_ite < source_size && t_ite < target_size)
        {
            // std::cout << s_ite << ' ' << source_size << ' ' << t_ite << ' '
            // << target_size << std::endl;

            if (source[s_ite] == target[t_ite])
            {
                ++s_ite;
                ++t_ite;
            }
            else
            {
                if (t_ite == 0)
                {
                    ++s_ite;
                }
                t_ite = k[t_ite];
            }
        }

        if (t_ite < target_size)
            pos = -1;
        else
            pos = s_ite - target_size;

        delete[] k;
        return pos;
    }
};