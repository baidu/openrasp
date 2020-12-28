/*
 * Copyright 2017-2021 Baidu Inc.
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
#include <cstdio>
#include <exception>

namespace openrasp
{

template <class T>
inline T _max(T x, T y) { return (x > y) ? x : y; }
template <class T>
inline T *_resize(T *ptr, size_t n, size_t l, T v)
{
  T *tmp = new T[l];
  for (size_t i = 0; i < n; ++i)
    tmp[i] = ptr[i];
  for (size_t i = n; i < l; ++i)
    tmp[i] = v;
  delete[] ptr;
  return tmp;
}

template <class array_type_, class array_u_type_>
class DoubleArrayImpl
{
public:
  typedef array_type_ value_type;
  typedef array_type_ result_type; // for compatibility

  struct result_pair_type
  {
    value_type value;
    size_t length;
  };

  explicit DoubleArrayImpl() : array_(0), used_(0),
                               size_(0), alloc_size_(0),
                               no_delete_(0), error_(0) {}

  virtual ~DoubleArrayImpl() { clear(); }

  void set_result(value_type *x, value_type r, size_t) const
  {
    *x = r;
  }

  void set_result(result_pair_type *x, value_type r, size_t l) const
  {
    x->value = r;
    x->length = l;
  }

  void set_array(void *ptr, size_t size = 0)
  {
    clear();
    array_ = reinterpret_cast<unit_t *>(ptr);
    no_delete_ = true;
    size_ = size;
  }

  const void *array() const
  {
    return const_cast<const void *>(reinterpret_cast<void *>(array_));
  }

  void clear()
  {
    if (!no_delete_)
      delete[] array_;
    delete[] used_;
    array_ = 0;
    used_ = 0;
    alloc_size_ = 0;
    size_ = 0;
    no_delete_ = false;
  }

  static constexpr size_t unit_size() { return sizeof(unit_t); }

  size_t size() const { return size_; }
  size_t total_size() const { return size_ * sizeof(unit_t); }

  size_t nonzero_size() const
  {
    size_t result = 0;
    for (size_t i = 0; i < size_; ++i)
      if (array_[i].check)
        ++result;
    return result;
  }

  int build(size_t key_size,
            const std::vector<std::string> *key,
            const size_t *length = 0,
            const std::vector<value_type> *value = 0)
  {
    if (!key_size || !key)
      return 0;

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

  inline result_pair_type match_search(const char *key,
                                       size_t len = 0,
                                       size_t node_pos = 0) const
  {
    if (!len)
      len = std::strlen(key);

    result_pair_type result;
    set_result(&result, -1, 0);

    register array_type_ b = array_[node_pos].base;
    register array_u_type_ p;

    for (register size_t i = 0; i < len; ++i)
    {
      p = b + (unsigned char)(key[i]) + 1;
      if (static_cast<array_u_type_>(b) == array_[p].check)
        b = array_[p].base;
      else
        return result;
    }

    p = b;
    array_type_ n = array_[p].base;
    if (static_cast<array_u_type_>(b) == array_[p].check && n < 0)
      set_result(&result, -n - 1, len);

    return result;
  }

  std::vector<result_pair_type> prefix_search(const char *key,
                                              size_t len = 0,
                                              size_t node_pos = 0) const
  {
    std::vector<result_pair_type> result;
    if (!len)
      len = std::strlen(key);

    register array_type_ b = array_[node_pos].base;
    register size_t num = 0;
    register array_type_ n;
    register array_u_type_ p;

    for (register size_t i = 0; i < len; ++i)
    {
      p = b; // + 0;
      n = array_[p].base;
      if ((array_u_type_)b == array_[p].check && n < 0)
      {
        result.push_back({-n - 1, i});
      }

      p = b + (unsigned char)(key[i]) + 1;
      if ((array_u_type_)b == array_[p].check)
        b = array_[p].base;
      else
        return result;
    }

    p = b;
    n = array_[p].base;

    if ((array_u_type_)b == array_[p].check && n < 0)
    {
      result.push_back({-n - 1, len});
    }

    return result;
  }

private:
  struct node_t
  {
    array_u_type_ code;
    size_t depth;
    size_t left;
    size_t right;
  };

  struct unit_t
  {
    array_type_ base;
    array_u_type_ check;
  };

  unit_t *array_;
  unsigned char *used_;
  size_t size_;
  size_t alloc_size_;
  size_t key_size_;
  const std::vector<std::string> *key_;
  const size_t *length_;
  const std::vector<array_type_> *value_;
  size_t progress_;
  size_t next_check_pos_;
  bool no_delete_;
  int error_;

  size_t resize(const size_t new_size)
  {
    unit_t tmp;
    tmp.base = 0;
    tmp.check = 0;
    array_ = _resize(array_, alloc_size_, new_size, tmp);
    used_ = _resize(used_, alloc_size_, new_size,
                    static_cast<unsigned char>(0));
    alloc_size_ = new_size;
    return new_size;
  }

  size_t fetch(const node_t &parent, std::vector<node_t> &siblings)
  {
    if (error_ < 0)
      return 0;

    array_u_type_ prev = 0;
    try
    {
      for (size_t i = parent.left; i < parent.right; ++i)
      {
        if ((length_ ? length_[i] : key_->at(i).length()) < parent.depth)
          continue;

        const unsigned char *tmp = reinterpret_cast<const unsigned char *>(key_->at(i).c_str());

        array_u_type_ cur = 0;
        if ((length_ ? length_[i] : key_->at(i).length()) != parent.depth)
          cur = (array_u_type_)tmp[parent.depth] + 1;

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
            siblings[siblings.size() - 1].right = i;

          siblings.push_back(tmp_node);
        }

        prev = cur;
      }
    }
    catch (const std::exception &e)
    {
      error_ = -4;
      return 0;
    }

    if (!siblings.empty())
      siblings[siblings.size() - 1].right = parent.right;

    return siblings.size();
  }

  size_t insert(const std::vector<node_t> &siblings)
  {
    if (error_ < 0)
      return 0;

    size_t begin = 0;
    size_t pos = _max((size_t)siblings[0].code + 1, next_check_pos_) - 1;
    size_t nonzero_num = 0;
    int first = 0;

    if (alloc_size_ <= pos)
      resize(pos + 1);

    while (true)
    {
    next:
      ++pos;

      if (alloc_size_ <= pos)
        resize(pos + 1);

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
        resize(static_cast<size_t>(alloc_size_ *
                                   _max(1.05, 1.0 * key_size_ / progress_)));

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
    size_ = _max(size_, begin + static_cast<size_t>(siblings[siblings.size() - 1].code + 1));

    for (size_t i = 0; i < siblings.size(); ++i)
      array_[begin + siblings[i].code].check = begin;

    try
    {
      /* code */
      for (size_t i = 0; i < siblings.size(); ++i)
      {
        std::vector<node_t> new_siblings;

        if (!fetch(siblings[i], new_siblings))
        {
          array_[begin + siblings[i].code].base =
              value_ ? static_cast<array_type_>(-value_->at(siblings[i].left) - 1) : static_cast<array_type_>(-siblings[i].left - 1);

          if (value_ && (array_type_)(-value_->at(siblings[i].left) - 1) >= 0)
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
    }
    catch (const std::exception &e)
    {
      error_ = -4;
      return 0;
    }

    return begin;
  }
};

using dat_value = long;
using dat_u_value = unsigned long;
using DoubleArrayTrie = openrasp::DoubleArrayImpl<dat_value, dat_u_value>;

} // namespace openrasp
#endif
