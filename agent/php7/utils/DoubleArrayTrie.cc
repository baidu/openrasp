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

#include "DoubleArrayTrie.h"

namespace openrasp
{
DoubleArrayTrie::DoubleArrayTrie()
    : array_(0), used_(0),
      size_(0), alloc_size_(0),
      no_delete_(0), error_(0)
{
}

DoubleArrayTrie::~DoubleArrayTrie()
{
    clear();
}

void DoubleArrayTrie::set_result(int *x, int r, size_t) const
{
    *x = r;
}

void DoubleArrayTrie::set_result(result_pair_type *x, int r, size_t l) const
{
    x->value = r;
    x->length = l;
}

void DoubleArrayTrie::load_existing_array(void *ptr, size_t size)
{
    clear();
    array_ = reinterpret_cast<unit_t *>(ptr);
    no_delete_ = true;
    size_ = size;
}

void DoubleArrayTrie::clear()
{
    if (!no_delete_)
    {
        delete[] array_;
    }
    delete[] used_;
    array_ = 0;
    used_ = 0;
    alloc_size_ = 0;
    size_ = 0;
    no_delete_ = false;
}

size_t DoubleArrayTrie::unit_size() const
{
    return sizeof(unit_t);
}

size_t DoubleArrayTrie::size() const
{
    return size_;
}

size_t DoubleArrayTrie::total_size() const
{
    return size() * unit_size();
}

size_t DoubleArrayTrie::nonzero_size() const
{
    size_t result = 0;
    for (size_t i = 0; i < size_; ++i)
    {
        if (array_[i].check)
            ++result;
    }
    return result;
}

const void *DoubleArrayTrie::array() const
{
    return const_cast<const void *>(reinterpret_cast<void *>(array_));
}

int DoubleArrayTrie::build(size_t key_size, const std::vector<std::string> *key, const size_t *length, const std::vector<int> *value)
{
    if (!key_size || !key)
    {
        return 0;
    }

    key_ = key;
    length_ = length;
    key_size_ = key_size;
    value_ = value;
    progress_ = 0;

    resize(8192);

    array_[0].base = 1;
    next_check_pos_ = 0;

    node_t root_node;
    root_node.left = 0;
    root_node.right = key_size;
    root_node.depth = 0;

    std::vector<node_t> siblings;
    fetch(root_node, siblings);
    insert(siblings);

    delete[] used_;
    used_ = 0;

    return error_;
}

size_t DoubleArrayTrie::prefix_search(const char *key, result_pair_type *result, size_t result_len, size_t len, size_t node_pos) const
{
    if (!len)
        len = std::strlen(key);

    register int b = array_[node_pos].base;
    register size_t num = 0;
    register int n;
    register unsigned int p;

    for (register size_t i = 0; i < len; ++i)
    {
        p = b; // + 0;
        n = array_[p].base;
        if ((unsigned int)b == array_[p].check && n < 0)
        {
            if (num < result_len)
            {
                set_result(&result[num], -n - 1, i);
            }
            ++num;
        }

        p = b + (unsigned char)(key[i]) + 1;
        if ((unsigned int)b == array_[p].check)
        {
            b = array_[p].base;
        }
        else
        {
            return num;
        }
    }

    p = b;
    n = array_[p].base;

    if ((unsigned int)b == array_[p].check && n < 0)
    {
        if (num < result_len)
            set_result(&result[num], -n - 1, len);
        ++num;
    }

    return num;
}

size_t DoubleArrayTrie::resize(const size_t new_size)
{
    unit_t tmp;
    tmp.base = 0;
    tmp.check = 0;
    array_ = array_resize(array_, alloc_size_, new_size, tmp);
    used_ = array_resize(used_, alloc_size_, new_size,
                         static_cast<unsigned char>(0));
    alloc_size_ = new_size;
    return new_size;
}

size_t DoubleArrayTrie::fetch(const node_t &parent, std::vector<node_t> &siblings)
{
    if (error_ < 0)
        return 0;

    unsigned int prev = 0;

    for (size_t i = parent.left; i < parent.right; ++i)
    {
        if ((length_ ? length_[i] : key_->at(i).length()) < parent.depth)
        {
            continue;
        }

        const unsigned char *tmp = reinterpret_cast<const unsigned char *>(key_->at(i).c_str());

        unsigned int cur = 0;
        if ((length_ ? length_[i] : key_->at(i).length()) != parent.depth)
            cur = (unsigned int)tmp[parent.depth] + 1;

        if (prev > cur)
        {
            error_ = -3;
            return 0;
        }

        if (cur != prev || siblings.empty())
        {
            node_t tmp_node;
            tmp_node.depth = parent.depth + 1;
            tmp_node.code = cur;
            tmp_node.left = i;
            if (!siblings.empty())
            {
                siblings[siblings.size() - 1].right = i;
            }

            siblings.push_back(tmp_node);
        }

        prev = cur;
    }

    if (!siblings.empty())
    {
        siblings[siblings.size() - 1].right = parent.right;
    }

    return siblings.size();
}

size_t DoubleArrayTrie::insert(const std::vector<node_t> &siblings)
{
    if (error_ < 0)
        return 0;

    size_t begin = 0;
    size_t pos = max((size_t)siblings[0].code + 1, next_check_pos_) - 1;
    size_t nonzero_num = 0;
    int first = 0;

    if (alloc_size_ <= pos)
    {
        resize(pos + 1);
    }

    while (true)
    {
    next:
        ++pos;

        if (alloc_size_ <= pos)
        {
            resize(pos + 1);
        }

        if (array_[pos].check)
        {
            ++nonzero_num;
            continue;
        }
        else if (!first)
        {
            next_check_pos_ = pos;
            first = 1;
        }

        begin = pos - siblings[0].code;
        if (alloc_size_ <= (begin + siblings[siblings.size() - 1].code))
            resize(static_cast<size_t>(alloc_size_ * max(1.05, 1.0 * key_size_ / progress_)));

        if (used_[begin])
            continue;

        for (size_t i = 1; i < siblings.size(); ++i)
            if (array_[begin + siblings[i].code].check != 0)
                goto next;

        break;
    }

    if (1.0 * nonzero_num / (pos - next_check_pos_ + 1) >= 0.95)
        next_check_pos_ = pos;

    used_[begin] = 1;
    size_ = max(size_, begin + static_cast<size_t>(siblings[siblings.size() - 1].code + 1));

    for (size_t i = 0; i < siblings.size(); ++i)
    {
        array_[begin + siblings[i].code].check = begin;
    }

    for (size_t i = 0; i < siblings.size(); ++i)
    {
        std::vector<node_t> new_siblings;

        if (!fetch(siblings[i], new_siblings))
        {
            array_[begin + siblings[i].code].base =
                value_ ? static_cast<int>(-value_->at(siblings[i].left) - 1) : static_cast<int>(-siblings[i].left - 1);

            if (value_ && (int)(-value_->at(siblings[i].left) - 1) >= 0)
            {
                error_ = -2;
                return 0;
            }

            ++progress_;
        }
        else
        {
            size_t h = insert(new_siblings);
            array_[begin + siblings[i].code].base = h;
        }
    }

    return begin;
}
} // namespace openrasp