/*
 * Copyright 2017-2019 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DOUBLE_ARRAY_TRIE_H_
#define _DOUBLE_ARRAY_TRIE_H_

#include <vector>
#include <cstring>
#include <string>
#include <cstdio>

namespace openrasp
{
template <class T>
inline T max(T x, T y) { return (x > y) ? x : y; }
template <class T>
inline T *array_resize(T *ptr, size_t origin_size, size_t new_size, T default_value)
{
    T *tmp = new T[new_size];
    for (size_t i = 0; i < origin_size; ++i)
    {
        tmp[i] = ptr[i];
    }
    for (size_t i = origin_size; i < new_size; ++i)
    {
        tmp[i] = default_value;
    }
    delete[] ptr;
    return tmp;
}

class DoubleArrayTrie
{
  public:
    struct result_pair_type
    {
        int value;
        size_t length;
    };

    struct unit_t
    {
        int base;
        unsigned int check;
    };

    explicit DoubleArrayTrie();
    virtual ~DoubleArrayTrie();
    void set_result(int *x, int r, size_t) const;
    void set_result(result_pair_type *x, int r, size_t l) const;
    void load_existing_array(void *ptr, size_t size = 0);
    void clear();
    size_t unit_size() const;
    size_t size() const;
    size_t total_size() const;
    size_t nonzero_size() const;
    int build(size_t key_size, const std::vector<std::string> *key, const size_t *length = 0, const std::vector<int> *value = 0);
    size_t prefix_search(const char *key, result_pair_type *result, size_t result_len, size_t len = 0, size_t node_pos = 0) const;
    const void *array() const;

  private:
    struct node_t
    {
        unsigned int code;
        size_t depth;
        size_t left;
        size_t right;
    };

    unit_t *array_;
    unsigned char *used_;
    size_t size_;
    size_t alloc_size_;
    size_t key_size_;
    const std::vector<std::string> *key_;
    const size_t *length_;
    const std::vector<int> *value_;
    size_t progress_;
    size_t next_check_pos_;
    bool no_delete_;
    int error_;

    size_t resize(const size_t new_size);
    size_t fetch(const node_t &parent, std::vector<node_t> &siblings);
    size_t insert(const std::vector<node_t> &siblings);
};

} // namespace openrasp

#endif