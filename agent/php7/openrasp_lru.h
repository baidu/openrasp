#pragma once

#include <unordered_map>
#include <list>
#include <functional>

namespace openrasp
{

template <typename T, typename U>
class LRU
{
private:
  struct Item
  {
    Item(size_t k, const U &v) : key_hash(k), value(v) {}
    size_t key_hash;
    U value;
  };
  std::hash<T> hasher;
  std::list<Item> item_list;
  std::unordered_map<size_t, typename list<Item>::iterator> item_map;
  size_t max;

  void reorder(const typename list<Item>::iterator it)
  {
    if (it == item_list.begin() && size() > max)
    {
      item_map.erase(item_list.back().key_hash);
      item_list.pop_back();
    }
    else
    {
      item_list.splice(item_list.begin(), item_list, it);
    }
  }

public:
  LRU(size_t max = 10) : max(max) {}

  typename list<Item>::iterator get(const T &key)
  {
    size_t key_hash = hasher(key);
    auto it = item_map.find(key_hash);
    if (it != item_map.end())
    {
      reorder(it->second);
      return it->second;
    }
    else
    {
      return item_list.end();
    }
  }
  void set(const T &key, const U &value)
  {
    if (max <= 0)
    {
      return;
    }
    size_t key_hash = hasher(key);
    auto it = item_map.find(key_hash);
    if (it != item_map.end())
    {
      if (it->second->value != value)
      {
        it->second->value = value;
      }
      reorder(it->second);
    }
    else
    {
      item_list.emplace_front(key_hash, value);
      auto it = item_list.begin();
      item_map.emplace(key_hash, it);
      reorder(it);
    }
  }

  bool contains(const T &key)
  {
    return get(key) != end();
  }
  bool empty() const
  {
    return item_list.empty();
  }
  void clear()
  {
    item_list.clear();
    item_map.clear();
  }
  void reset(size_t max = 10)
  {
    clear();
    this->max = max;
  }
  size_t size() const
  {
    return item_list.size();
  }
  size_t max_size() const
  {
    return max;
  }
  typename list<Item>::iterator begin()
  {
    return item_list.begin();
  }
  typename list<Item>::iterator end()
  {
    return item_list.end();
  }
};
} // namespace openrasp