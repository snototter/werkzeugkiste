#ifndef __WERKZEUGKISTE_CONTAINER_SORT_H__
#define __WERKZEUGKISTE_CONTAINER_SORT_H__

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <exception>
#include <sstream>


namespace werkzeugkiste {
//TODO doc
namespace container {
/** @brief Extracts the keys from a map container. */
template <typename M>
std::vector<typename M::key_type> GetMapKeys(const M &map) {
  std::vector<typename M::key_type> vec;
  vec.reserve(map.size());
  for (typename M::const_iterator it = map.begin();
       it != map.end(); ++it) {
    vec.push_back(it->first);
  }
  return vec;
}


/** @brief Sort comparator using `operator<`. */
template <typename _T>
bool CmpAsc(const _T &a, const _T &b) {
  return a < b;
}


/** @brief Sort comparator using `operator<`. */
template <typename _T>
bool CmpDesc(const _T &a, const _T &b) {
  return b < a;
}


/** @brief Utility class to get the ordering (i.e. the sorted indices) of a vector. */
template <typename _T>
class Ordering {
 public:
  Ordering() = delete;

  Ordering(const std::vector<_T> &data)
    : data_(data), cmp_(CmpAsc<_T>)
  {}

  Ordering(const std::vector<_T> &data,
           bool (*cmp)(const _T &, const _T &))
    : data_(data), cmp_(cmp)
  {}

  std::vector<size_t> GetSortedIndices() {
    InitIndices();
    std::sort(indices_.begin(), indices_.end(), *this);
    return indices_;
  }

  bool operator()(size_t const &a, size_t const &b) {
    return cmp_(data_[a], data_[b]);
  }

 private:
  const std::vector<_T> &data_;
  std::vector<size_t> indices_;
  bool (*cmp_)(const _T &, const _T &);

  void InitIndices() {
    indices_.clear();
    indices_.reserve(data_.size());
    for (size_t i = 0; i < data_.size(); ++i) {
      indices_.push_back(i);
    }
  }
};


/** @brief Returns the indices corresponding to a sorted data vector. */
template <typename _T>
std::vector<size_t> GetSortedIndices(const std::vector<_T> &data,
                                     bool (*cmp)(const _T &, const _T &) = CmpAsc<_T>) {
  Ordering<_T> ordering(data, cmp);
  return ordering.GetSortedIndices();
}


/** @brief Remaps the data vector by the given indices. */
template <typename _T>
std::vector<_T> ApplyIndexLookup(const std::vector<_T> &data,
                                 const std::vector<size_t> &indices) {
  std::vector<_T> remapped;
  remapped.reserve(data.size());
  for (size_t i = 0; i < data.size(); ++i) {
    remapped.push_back(data[indices[i]]);
  }
  return remapped;
}


/** @brief Returns the data vector sorted by the given external keys. */
template <typename _Td, typename _Tk>
std::vector<_Td> SortByExternalKeys(const std::vector<_Td> &data,
                                    const std::vector<_Tk> &keys,
                                    bool (*cmp)(const _Tk &, const _Tk &) = CmpAsc<_Tk>) {
  if (data.empty()) {
    return std::vector<_Td>();
  }

  if (keys.size() != data.size()) {
    std::ostringstream s;
    s << "Vector size mismatch, " << data.size()
      << " vs " << keys.size() << "!";
    throw std::invalid_argument(s.str());
  }

  // Sort the indices
  const std::vector<size_t> indices = GetSortedIndices<_Tk>(keys, cmp);

  // Remap the input vector
  return ApplyIndexLookup<_Td>(data, indices);
}


//TODO more general sort, i.e. C Sort(C, cmp)
///** @brief Returns the sorted vector (handy if you don't want to change the input data. */
//template <typename _T>
//std::vector<_T> SortVector(const std::vector<_T> &data,
//                           bool (*cmp)(const _T &, const _T &) = CmpAsc<_T>) {
//  // Copy, then sort
//  std::vector<_T> copy;
//  copy.reserve(data.size());
//  for (const _T &e : data) {
//    copy.push_back(e);
//  }
//  std::sort(copy.begin(), copy.end(), cmp);
//  return copy;
//}
//template<typename C,
//    typename T = std::decay_t<
//        decltype(*begin(std::declval<C>()))>>
//C Sort(const C &container,
//       bool (*cmp)(const T &, const T &) = CmpAsc<T>) {
//  // Copy, then sort:
//  C copy;
//  for (const auto &e : container) {
//    copy
//  }
//  return copy;
//}
//TODO Sort:
// std::sort requires randomaccess (

//TODO sfinae to accept all iterable containers
/** @brief Finds all duplicate entries in 'data' and stores their frequencies in item_count. */
//template <typename _T>
//std::map<_T, std::size_t> FindDuplicates(const std::vector<_T> &data) {
template<typename C,
    typename T = std::decay_t<
        decltype(*begin(std::declval<C>()))>>
std::map<T, std::size_t> FindDuplicates(const C &container) {
  std::map<T, std::size_t> item_count;
  std::map<T, std::size_t> item_count_tmp;

  // Compute frequency for each item
  for (const auto &item : container) {
    auto itmp = item_count_tmp.insert(std::pair<T, size_t>(item, 1));
    // If insertion fails, the key existed already
    if (itmp.second == false) {
      itmp.first->second++;
    }
  }

  // We need to return only duplicates:
  for (auto it = item_count_tmp.begin();
       it != item_count_tmp.end(); it++) {
    if (it->second > 1) {
      item_count.insert(*it);
    }
  }
  return item_count;
}

/**
 * @brief Returns true if there are no duplicates
 * in the given vector. Simply a convienience wrapper
 * to @see `FindDuplicates()`.
 */
//TODO iterable
// sfinae testing value_type & begin/end functionality
//   https://devblogs.microsoft.com/oldnewthing/20190619-00/?p=102599
// e.g. test for ::key_type https://en.cppreference.com/w/cpp/named_req/AssociativeContainer
// or specialized is_xy template: https://stackoverflow.com/questions/22156957/detect-whether-type-is-associative-container
// general template with specialization for each class
//   https://stackoverflow.com/questions/42485829/c-stl-stdfind-with-stdmap
//
//template <typename _T>
//bool HasUniqueItems(const std::vector<_T> &data) {
template<typename C,
    typename T = std::decay_t<
        decltype(*begin(std::declval<C>()))>>
bool HasUniqueItems(const C &container) {
  auto duplicates = FindDuplicates(container);
  return duplicates.empty();
}


////TODO doc
//template <typename _T, typename Iter>
//bool Contains(const _T &value, Iter begin, Iter end) {
//  return std::find(begin, end, value) != end;
//}

//template <typename _T>
//bool Contains(const std::vector<_T> &container, const _T &value) {
//  return std::find(container.cbegin(), container.cend(),
//                   value) != container.cend();
//}

//TODO contains_key/contains_value?
template <typename _Tv, typename... _Ts>
bool ContainsKey(const std::map<_Ts...> &container, const _Tv &key) {
  return container.find(key) != std::end(container);
}


////SFINAE :)
//// see https://devblogs.microsoft.com/oldnewthing/20190619-00/?p=102599
//template<typename C,
//         typename T = std::decay_t<
//           decltype(*begin(std::declval<C>()))>>
//bool Contains(const C &container, const T &value) {
//  return std::find(container.begin(), container.end(), value) != container.end();
//}

//TODO check the detection idiom and proper usage of sfinae
//but watch out for caveats, like using ::is_pair can also be true
// for array/vector which contains pairs...
// e.g. https://stackoverflow.com/a/42485895/400948




}  // namespace container
}  // namespace werkzeugkiste

#endif  // __WERKZEUGKISTE_CONTAINER_SORT_H__
