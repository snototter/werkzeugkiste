#ifndef WERKZEUGKISTE_CONTAINER_SORT_H
#define WERKZEUGKISTE_CONTAINER_SORT_H

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <exception>
#include <sstream>


namespace werkzeugkiste {
/// Utility functions for standard containers.
namespace container {

/// Returns the keys from a map container (i.e. any
/// associative container).
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


/// A sort comparator for ascending order, which uses `operator<`.
template <typename Tp>
bool CmpAsc(const Tp &a, const Tp &b) {
  return a < b;
}


/// A sort comparator for descending order, which uses `operator<`.
template <typename Tp>
bool CmpDesc(const Tp &a, const Tp &b) {
  return b < a;
}


/// Utility class to get the sorted indices
/// of a sequence container.
template <class Container>
class Ordering {
public:
  Ordering() = delete;

  Ordering(
      const Container &data,
      bool (*cmp)(
        const typename Container::value_type &,
        const typename Container::value_type &))
    : data_(data), cmp_(cmp)
  {}

  Ordering(const Container &data)
    : Ordering(data, CmpAsc<typename Container::value_type>)
  {}

  std::vector<std::size_t> GetSortedIndices() {
    InitIndices();
    std::sort(indices_.begin(), indices_.end(), *this);
    return indices_;
  }

  bool operator()(std::size_t const &a, std::size_t const &b) {
    return cmp_(data_[a], data_[b]);
  }

private:
  const Container &data_;
  std::vector<std::size_t> indices_;
  bool (*cmp_)(
      const typename Container::value_type &,
      const typename Container::value_type &);

  void InitIndices() {
    indices_.clear();
    indices_.reserve(data_.size());
    for (std::size_t i = 0; i < data_.size(); ++i) {
      indices_.push_back(i);
    }
  }
};


/// Returns the indices which correspond to a sorted `data` vector.
template <class Container>
std::vector<std::size_t> GetSortedIndices(
      const Container &data,
      bool (*cmp)(
          const typename Container::value_type &,
          const typename Container::value_type &
      ) = CmpAsc<typename Container::value_type>) {
  Ordering<Container> ordering(data, cmp);
  return ordering.GetSortedIndices();
}


/// Returns a container obtained by remapping the
/// given `data` according to the `indices`.
///
/// Requires the input to be a sequence container,
/// i.e. it must provide `push_back`.
template <class Container>
Container ApplyIndexLookup(
    const Container &data,
    const std::vector<std::size_t> &indices) {
  Container remapped;
  // TODO For a more generic template, I need to dig deeper
  // into container types and maybe SFINAE: `reserve` is not
  // required by the sequence container interface...
//  remapped.reserve(data.size());
  for (std::size_t i = 0; i < indices.size(); ++i) {
    remapped.push_back(data[indices[i]]);
  }
  return remapped;
}


/// Returns the data vector sorted by the given external keys.
template <typename TData, typename TKey>
std::vector<TData> SortByExternalKeys(
    const std::vector<TData> &data,
    const std::vector<TKey> &keys,
    bool (*cmp)(const TData &, const TKey &) = CmpAsc<TKey>) {
  if (data.empty()) {
    return std::vector<TData>{};
  }

  if (keys.size() != data.size()) {
    std::ostringstream s;
    s << "Number of keys (" << keys.size()
      << ") must equal the number of data items (" << data.size() << ")!";
    throw std::invalid_argument(s.str());
  }

  // Sort the indices
  const std::vector<std::size_t> indices = GetSortedIndices<TKey>(keys, cmp);

  // Remap the input vector
  return ApplyIndexLookup<TData>(data, indices);
}


/// Returns a map containing all duplicate entries in 'data' along
/// with their their frequencies.
template<typename Container,
    typename Tp = std::decay_t<
        decltype(*begin(std::declval<Container>()))>>
std::map<Tp, std::size_t> FindDuplicates(const Container &container) {
  std::map<Tp, std::size_t> item_count;
  std::map<Tp, std::size_t> item_count_tmp;

  // Compute frequency for each item
  for (const auto &item : container) {
    auto itmp = item_count_tmp.insert(std::pair<Tp, std::size_t>(item, 1));
    // If insertion fails, the key existed already
    if (itmp.second == false) {
      itmp.first->second++;
    }
  }

  // We need to return only duplicates:
  for (auto it = item_count_tmp.begin(); it != item_count_tmp.end(); it++) {
    if (it->second > 1) {
      item_count.insert(*it);
    }
  }
  return item_count;
}


/// Returns true if there are no duplicates in the given
/// sequence container.
template<typename Container,
    typename Tp = std::decay_t<
        decltype(*begin(std::declval<Container>()))>>
bool HasUniqueItems(const Container &container) {
  auto duplicates = FindDuplicates(container);
  return duplicates.empty();
}


/// Returns true if the given key exists within the map.
template <typename Tv, typename... Ts> inline
bool ContainsKey(const std::map<Ts...> &container, const Tv &key) {
  return container.find(key) != std::end(container);
}


/// Returns true if the given element exists within the container.
template<typename Container,
    typename Tp = std::decay_t<
        decltype(*begin(std::declval<Container>()))>>
bool ContainsValue(const Container &container, const Tp &value) {
  return std::find(
        container.begin(), container.end(), value) != container.end();
}


}  // namespace container
}  // namespace werkzeugkiste

#endif  // WERKZEUGKISTE_CONTAINER_SORT_H
