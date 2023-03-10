#pragma once

#include <vector>
#include <string>
#include "AudioTypes.h"

/**
 * @brief Splits a string according to a delimiter
 * 
 * @param s 
 * @param delimiter 
 * @return std::vector<std::string> 
 */
std::vector<std::string> split(const std::string &s, const std::string &delimiter);

/**
 * @brief
 *
 * @param outwav
 * @param content
 * @param fs
 * @param bps
 * @param nch
 */
void write_wav(const std::string &outwav, const std::vector<float32_t> &content,
               cint32_t fs = 48000, cint32_t bps = 16, cint32_t nch = 1);

/**
 * @brief 
 * 
 * @param wavL 
 * @param wavR 
 * @param eps 
 * @return true 
 * @return false 
 */
bool compare_wav(const std::string &wavL, const std::string &wavR,
                 float32_t eps = 1e-9);

/**
 * @brief 
 * 
 * @param left 
 * @param right 
 * @param eps 
 * @return true 
 * @return false 
 */
bool compare_csv(const std::string &left, const std::string &right, float32_t eps = 1e-9);

/**
 * @brief 
 * 
 * @tparam T 
 * @param v 
 * @param n 
 * @return std::vector<std::vector<T>> 
 */
template <typename T>
std::vector<std::vector<T>> split(std::vector<T> &v, int32_t n)
{
    std::vector<std::vector<float32_t>> vec_split;
    cint32_t niter = static_cast<cint32_t>(ceil(v.size() / (float32_t)n));
    v.resize(niter * n);
    for (auto i = 0; i < niter; i++)
    {
        auto first = v.begin() + n * i;
        auto last = v.begin() + n * (i + 1);
        std::vector<float32_t> vec(first, last);
        vec_split.push_back(vec);
    }

    return vec_split;
}

/**
 * @brief 
 * 
 * @tparam T 
 * @param v 
 * @param n 
 * @return std::vector<std::vector<T>> 
 */
template <typename T>
std::vector<std::vector<T>> split(std::vector<T> &v, std::vector<int32_t> n)
{
    std::vector<std::vector<float32_t>> vec_split;
    int32_t sum = 0;
    for (auto it = n.begin(); it != n.end(); it++)
        sum += (*it);
    v.resize(ceil(v.size() / (float32_t)sum) * sum);
    int32_t offset = 0, i = 0;
    while (offset < v.size())
    {
        auto len = n[i % n.size()];
        auto first = v.begin() + offset;
        auto last = v.begin() + offset + len;
        std::vector<float32_t> vec(first, last);
        vec_split.push_back(vec);
        offset += len;
        i++;
    }

    return vec_split;
}
