//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <utility>
#include "common/config.h"
#include "common/exception.h"
#include "type/limits.h"

namespace bustub {

static const size_t INF_TIMESTAMP = std::numeric_limits<size_t>::max();

LRUKNode::LRUKNode(frame_id_t fid, size_t k) : k_(k), fid_(fid) {}
auto LRUKNode::GetFrameId() const -> frame_id_t { return fid_; }
auto LRUKNode::IsEvictable() const -> bool { return is_evictable_; }
void LRUKNode::SetEvictable(bool set_evictable) { is_evictable_ = set_evictable; }
// auto LRUKNode::GetEarliestTimestamp() const->size_t {return history_.front();}
auto LRUKNode::GetHistory() const -> std::list<size_t> { return history_; }
void LRUKNode::SetHistory(size_t timestamp) { history_.push_front(timestamp); }

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  if (node_store_.empty() || curr_size_ == 0) {
    return false;
  }
  size_t min_least_recent = INF_TIMESTAMP;
  size_t max_k_distance = 0;
  bool evict = false;
  latch_.lock();
  if (node_store_.empty() || curr_size_ == 0) {
    return false;
  }
  for (auto &store : node_store_) {
    if (store.second.IsEvictable()) {
      auto node = store.second;
      size_t least_recent = node.GetHistory().front();
      size_t k_least_recent = node.GetHistory().back();
      if (node.GetHistory().size() < k_) {
        max_k_distance = INF_TIMESTAMP;
        if (k_least_recent < min_least_recent) {
          min_least_recent = least_recent;
          *frame_id = node.GetFrameId();
          evict = true;
        }
      } else if (max_k_distance != INF_TIMESTAMP) {
        size_t count = 1;
        for (size_t history : node.GetHistory()) {
          if (count == k_) {
            k_least_recent = history;
          }
          count++;
        }
        if (max_k_distance < least_recent - k_least_recent) {
          max_k_distance = least_recent - k_least_recent;
          *frame_id = node.GetFrameId();
          evict = true;
        }
      }
    }
  }
  if (evict) {
    node_store_.erase(*frame_id);
    curr_size_--;
  }
  latch_.unlock();
  return evict;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  if (frame_id < 0 || static_cast<size_t>(frame_id) >= replacer_size_) {
    throw std::invalid_argument{"invalid frame id"};
  }
  latch_.lock();
  auto it = node_store_.find(frame_id);
  if (it == node_store_.end()) {
    LRUKNode node(frame_id, k_);
    node.SetHistory(current_timestamp_);
    node_store_.insert(std::make_pair(frame_id, node));
  } else {
    it->second.SetHistory(current_timestamp_);
  }
  current_timestamp_++;
  latch_.unlock();
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  if (frame_id < 0 || static_cast<size_t>(frame_id) >= replacer_size_) {
    throw std::invalid_argument("invalid frame id");
  }
  latch_.lock();
  auto it = node_store_.find(frame_id);
  if (it == node_store_.end()) {
    return;
  }
  if (it->second.IsEvictable() && !set_evictable) {
    it->second.SetEvictable(false);
    curr_size_--;
  } else if (!it->second.IsEvictable() && set_evictable) {
    it->second.SetEvictable(true);
    curr_size_++;
  }
  latch_.unlock();
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  latch_.lock();
  auto it = node_store_.find(frame_id);
  if (it == node_store_.end()) {
    return;
  }
  if (it->second.IsEvictable()) {
    node_store_.erase(frame_id);
    replacer_size_--;
  } else {
    throw std::invalid_argument{"invalid frame id"};
  }
  latch_.unlock();
}

auto LRUKReplacer::Size() -> size_t { return curr_size_; }

}  // namespace bustub
